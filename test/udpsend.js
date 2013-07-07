var dgram = require("dgram");

var server = dgram.createSocket("udp4");

var msg = new Buffer(process.argv.slice(2).join(' '));

server.bind(4444, function () {
  server.send(msg, 0, msg.length, 2390, '192.168.1.117', function(err, bytes) {
    console.log('SENT', err, bytes);
    server.close();
  });
});