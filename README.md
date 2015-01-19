FrigoTunnel
===========

A small library to get a simple communication channel between devices on a same
network. Just set a name to each device, and you can send messaes to them
without worrying about their IP address.

Typical Use
-----------

Here is some short snippet that demonstrates how to send and receive messages
using FrigoTunnel.

```cpp
FrigoTunnel tunnel("my-host");

connect(&tunnel, &FrigoTunnel::gotMessage, [=](const QJsonObject &message) {
  qDebug() << "inbound message" << message;
});

QJsonObject content;
content["foo"] = "bar";

FrigoMessage message(content);
message.to("some-other-host");

FrigoPacket packet(&message);
tunnel.send(&packet);
```

In the above example, the message is sent to the host `some-other-host`, however
you could also broadcast it using the special hostname `*`.

Internals
---------

Basically, each node listens to the same UDP multicast address (`225.42.42.42`)
and waits for messages addressed to them to come.

To improve reliability, there is also node discovery system. Each node
broadcasts its name every few seconds (a random interval, between 10 and 30
seconds), and upon receiving a name, a TCP socket is opened.

When a regular message is sent, it is sent first through the UDP socket, and
then through each currently open TCP socket, in case the UDP packet didn't make
it.

Changelog
---------

### Version 1.0.0

- Initial Public Version
- Can send/receive reliable unicast or multicast messages

Copyright
---------

This project is written by Rémy Sanchez <<remy@activkonnect.com>>, and is
available under the terms of the WTFPL license, which is included in this
repository.
