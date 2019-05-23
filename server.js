const express = require('express'), 
	net = require('net'), 
	path = require('path'); 

const app = express(); 

app.set('views', path.join(__dirname, 'views')); 
app.set('view engine', 'ejs');

app.use(express.json());
app.use(express.urlencoded({ extended: false }));
app.use(express.static(path.join(__dirname, 'public')));

app.get('/', function(req, res) {
    res.render('index');
});

app.post('/play', function(req, res) {
	let key = req.body.key;
	console.log(key);
	
	if (isConnected) {
		console.log(`${key} has sent`);
		espConnection.write(`${key}\r\n`);
		res.status(200).end('<< MCU is ready >>');
	} else {
		res.status(200).end('<< MCU is connecting ... >>');
	}
});

app.listen(8000);

let espConnection = null;
let isConnected = false;
const netServer = net.createServer(function(connection) {
	console.log('esp8266 connected');
	espConnection = connection;
	isConnected = true;

	espConnection.on('close', function(err) {
		isConnected = false;
		console.log('Close because', err);
	});

	espConnection.on('end', function() {
		isConnected = false;
		console.log('esp 8266 Disconnected');
	});

	espConnection.on('error', function(err) {
		isConnected = false;
		console.log('Error when trying to connect', err.message);
	});
});

// netServer.maxConnections = 1;

netServer.listen(8124, function() {
	console.log('Listening for esp8266 connecting to port 8124...');
});

