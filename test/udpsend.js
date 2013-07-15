var dgram = require("dgram");

var server = dgram.createSocket("udp4");

var msg = new Buffer(process.argv.slice(2).join(' ') || 'test');

server.bind(4444, function () {
  server.send(msg, 0, msg.length, 2390, '10.1.90.49', function(err, bytes) {
    console.log('SENT', err, bytes);
    server.close();
  });
});