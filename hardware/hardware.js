const { SerialPort } = require('serialport');
const { ReadlineParser } = require('@serialport/parser-readline');

const SERIAL_PATH = 'COM5'; //  порт з Arduino IDE

const port = new SerialPort({
  path: SERIAL_PATH,
  baudRate: 9600,
});

const parser = port.pipe(new ReadlineParser({ delimiter: '\n' }));

port.on('open', () => {
  console.log(`[Serial] Порт ${SERIAL_PATH} відкрит. Чекаю данні .... `);
});

port.on('error', (err) => {
  console.error('[Serial] Помилка порту:', err.message);
});

parser.on('data', (rawLine) => {
  const line = rawLine.trim();
  if (!line) return;

  let sensorData;
  try {
    sensorData = JSON.parse(line);
  } catch (err) {
    console.warn('[Parser] Пропущено збитий кадр:', line);
    return;
  }

  const isValid =
    typeof sensorData.temperature === 'number' &&
    typeof sensorData.humidity === 'number' &&
    typeof sensorData.light === 'number';

  if (!isValid) {
    console.warn('[Parser] Неповні данні:', sensorData);
    return;
  }

  console.log('[Arduino -> Node.js]', sensorData);
});