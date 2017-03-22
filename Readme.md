This is an implementation for Arduino of the protocol described in [Very Low-Cost Sensing and Communication Using Bidirectional LEDs](http://math.hws.edu/vaughn/cpsc/336/docs/led-sensor.pdf) by Paul Dietz, William Yerazunis, and Darren Leigh.

It requires two I/O pins (currently `3` and `4`), an LED, and a resistor, hooked up as described in the paper. In addition, two other pins are used as indicators: pin `13` is pulled high to indicate an active connection to another device speaking the iDropper protocol, and pin `11` can have its brightness controlled by the remote device.

# Code Architecture
The code is arranged along vaguely OSI-esque lines, like so:

<table>
<thead><tr>
<th>Layer</th><th>File</th><th>Sending</th><th>Receiving</th>
</tr></thead>
<tr><td>Byte</td>       <td><a href="iDropper.ino">iDropper.ino</a></td>                    <td>idrop_next_byte()</td><td>idrop_recv_byte()</td></tr>
<tr><td>Bit</td>        <td><a href="idrop_3_bit.ino">idrop_3_bit.ino</a></td>              <td>idrop_next_bit()</td> <td>idrop_recv_bit()</td></tr>
<tr><td>Connection</td> <td><a href="idrop_2_connection.ino">idrop_2_connection.ino</a></td><td>idrop_send()</td>     <td>idrop_recv()</td></tr>
<tr><td>I/O</td>        <td><a href="idrop_1_io.ino">idrop_1_io.ino</a></td>                <td>idrop_emit()</td>     <td>idrop_detect()</td></tr>
<tr><td colspan=2>Physical</td>   <td colspan=2>LED & resistor</td></tr>
</table>

## Byte layer
Bytes (properly *octets*) are the highest-level layer currently handled. You could of course write higher layers with framing, packets, etc; I simply haven't bothered to do so. The protocol currently implemented is to continuously send the current value of Analog input `1` as a byte between ASCII '0' and '9', and receive a similar value any write it to PWM 11. Any recieved bytes not between ASCII `0`-`9` are echoed to the serial output.

When a byte is received, `idrop_recv_byte(byte b)` is called with the value of that byte. Conversely, when a connection has been established and a byte is ready to be sent `idrop_next_byte(boolean &idle, byte &resp)` is called to obtain the value to be sent. `idle` should be set to indicate that there are no bytes pending to be sent.

This general pattern of lower layers pushing received data to and pulling data to be sent from higher layers is continued throughout the code.

## Bit layer
`idrop_recv_bit()` is called when individual bits are received, and packs them into bytes. It has a small state machine with four states, as follows:
* `0`: Ready to receive and skip first bit.
* `1`: First bit skipped, idling on marks ready to begin byte when framing space is seen.
* `2`: Currently receiving byte.
* `3`: All 8 bits received; waiting for framing mark at end of byte.

`idrop_send_bit()` similarly has logic to unpack bytes and apply framing.

The bit layer is not aware of the state of the connection; if the connection drops in the middle of a byte then framing errors will be generated on the first byte after the connection resumes. The byte layer is expected to be aware of the unreliability of this connection and compensate.

## Connection layer
The connection layer converts logical bits into physical timings and vice versa. It handles the low-level link protocol to detect a peer iDropper device. Ordinarily, it idles sending marks waiting for a peer. When a peer device connects and completes the handshake, it begins transferring bits until the peer device disconnects again, at which point it returns to idling.

The global variable `idrop_connected` is maintained by `idrop_recv()`, and indicates whether there is currently a peered iDropper on the other end of the connection. If this value is true then a bit stream is active; if false then layers above the connection layer will not be driven.

`idrop_send()` simply retrieves a bit from the bit layer and transmits it if there is an active connection.

`idrop_recv()` is split into two functions, `idrop_recv_connected()` and `idrop_recv_idle()` based on whether there is an active connection. `idrop_recv_idle()` looks for a stream of marks from a peer device. Under most circumstances it will return quickly but if there is something that looks like a peer it may pause up to four seconds (as set by a timing loop). `idrop_recv_connected()` handles the rather complex series of timings required to reliably receive a bit. It may take up to four milliseconds (again, set by timing loop) if the peer connection drops.

`idrop_loop()` is also defined at this layer. It handles driving the protocol and running the communications, and should be called continuously to maintain the connection. In total, it may pause for up to four milliseconds (though usually far less than that, unless the state of the connection changes) in the bit layer and below, or however long the byte layer and above take to run.

## I/O Layer
The I/O layer handles actually driving the hardware. `idrop_emit()` just sets the pin to an output and turns the LED on or off; `idrop_detect()` handles the dance of setting the LED up as an input and driving it in the correct sequence to obtain a threshold value.
