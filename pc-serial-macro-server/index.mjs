import { SerialPort } from 'serialport';
import nodeChron from 'node-cron';
import { exec } from "node:child_process";
import * as fs from 'fs';

const portName = process.argv[2];
const trustedDevices = ["E7-6A-E0-D3-61-87"];
const platform = process.platform;

// load commands for current OS
let commands = {};
switch (platform) {
  case "win32": // Windows
    commands = JSON.parse(fs.readFileSync("./commands/windows.json", "utf8"));
    break;
  case "linux": // Linux
    commands = JSON.parse(fs.readFileSync("./commands/linux.json", "utf8"));
    break;
  case "darwin": // Mac
    commands = JSON.parse(fs.readFileSync("./commands/mac.json", "utf8"));
    break;
  default:
    console.error("OS not supported");
    process.exit(1);
}

function sleep(ms) {
  return new Promise((resolve) => {
    setTimeout(resolve, ms);
  });
}

// check if the key press is coming from known device
function isTrusted(data) {
  return trustedDevices.some((mac) => data.includes(mac));
}

function handleCommand(data) {
  try {
    let command = data.split(":");
    if (command.length < 3) {
      return;
    }
    command = parseInt(command[2]);

    // volume knob inputs range from 5000 to 5100
    if (command >= 5000 && command <= 5100) {
      command -= 5000;
      switch (platform) {
        case "win32":
          let volume = Math.ceil(command * 65535 / 100);
          command = `${commands["set_volume"]} ${volume} ${volume}`;
          break;
        case "linux":
		  command = Math.ceil(command * 1.5);
          command = `${commands["set_volume"]} ${command}%`;
          break;
        case "darwin":
          break;
      }
    } else {
      command = commands[command];
    }

    console.log(`\t${command}`);
    if(command) {
      const process = exec(command, (err) => {
        if (err) {
          console.error("\t could not execute command: ", err);
          return;
        }
        port.write("1");
      });
    }
  } catch (err) {
    console.error(`\tError processing command: ${data}`, err);
  }
}

// open the port
const port = new SerialPort({
  path: portName,
  baudRate: 115200,
});
console.log("Listening port: " + portName);

// handle data when it arrives
port.on('data', (d) => {
  const data = d.toString().replace("\n", "");

  // ignore logs and other messages
  if (!data.startsWith("got:")) {
    return;
  }

  console.log(new Date().toLocaleTimeString() + "> " + data);
  if (isTrusted(data)) {
    handleCommand(data);
  } else {
    console.warn("Unknown device!!");
  }
});

const task = nodeChron.schedule('0 * * * *', () => {
  const hour = new Date().getHours();
  if (hour > 6 && hour < 21) {
    // hourly chime
    port.write("2");
  } else if (hour == 21) {
    // call it a day
    port.write("3");
  }
});

//port.close();