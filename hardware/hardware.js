const { SerialPort } = require('serialport');
const { ReadlineParser } = require('@serialport/parser-readline');
const axios = require('axios');

const SERIAL_PATH = process.env.SERIAL_PORT || 'COM5'; //  порт з Arduino IDE
const API_URL = process.env.API_URL || 'http://localhost:3000'; //- базова адреса бекенд-сервера

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

parser.on('data', async(rawLine) => {          // - оскільки всередині використовуємо await axios.post;
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
    console.warn('[Parser] Невірний тип данних:', sensorData);
    return;
  }

  console.log('[Arduino -> Node.js]', sensorData);

  try {
    const response = await axios.post(`${API_URL}/api/weather`, sensorData, {
      timeout: 5000,
    });
    console.log(`[API -> Server] Збережено (HTTP ${response.status}):`, response.data);
  } catch (err) {
    console.error('[API] Не вдалося відправити дані:', err.message);
  }
});