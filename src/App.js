import React, {useState, useRef} from 'react';
import { ResponsiveLine } from '@nivo/line'

import logo from './logo.svg';
import './App.css';

const nivodata = require('./nivodata.js');

const service_uuid_led = '19b10000-e8f2-537e-4f6c-d104768a1214';
const service_uuid_acc = '19b10100-e8f2-537e-4f6c-d104768a1214';
const service_uuid_update = '19b10400-e8f2-537e-4f6c-d104768a1214';

const char_uuid_switch = '19b10001-e8f2-537e-4f6c-d104768a1214';
const char_uuid_acc_x = '19b10101-e8f2-537e-4f6c-d104768a1214';
const char_uuid_acc_y = '19b10102-e8f2-537e-4f6c-d104768a1214';
const char_uuid_acc_z = '19b10103-e8f2-537e-4f6c-d104768a1214';
const char_uuid_acc_t = '19b10104-e8f2-537e-4f6c-d104768a1214';
const char_uuid_update_period = '19b10401-e8f2-537e-4f6c-d104768a1214';

const dec = new TextDecoder('utf-8');
const enc = new TextEncoder('utf-8');

const Bluetooth = (props) => {

  const [device, setDevice] = useState(undefined);

  const removeDevice = () => {

    console.log('Disconnecting from Bluetooth Device...');
    if (device.info.gatt.connected) {
      device.info.gatt.disconnect();
    } else {
      console.log('> Bluetooth Device is already disconnected');
    }

  }

  const addDevice = () => {
    if(typeof(device) !== 'undefined'){
      console.warn('already connected');
      return;
    }
  
    var dev = {};
    var characteristics = {};
  
    return navigator.bluetooth.requestDevice({
      filters: [
        {name: 'Artemis'},
      ],
      optionalServices: [                   // list any other uuids that you want to have permission to access here
        service_uuid_led,
        service_uuid_acc,
        service_uuid_update,
      ]
    })
    .then(device => {
      device.addEventListener('gattserverdisconnected', (e) => {
        setDevice(undefined);
        props.onDisconnect(e);
      });
      dev['info'] = device;
      return device.gatt.connect();
    })
    .then(server => {
      dev['server'] = server;
      return server.getPrimaryServices();
    })
    .then(services => {
      dev['services'] = services;
      return services.map(s => s.getCharacteristics());
    })
    .then(characteristic_promises => {
      return Promise.all(characteristic_promises);
    })
    .then( service_characteristics => {
      service_characteristics.map(s => {    // for each service s
        s.map(c => {                        // for each characteristic c in s
          characteristics[c.uuid] = c;      // cache the characteristic
        });
      });
    })
    .then(_ => {                            // add the device to the state
      dev['characteristics'] = characteristics;
      setDevice(dev);

      // read the LED characteristic value
      console.log('reading LED state');

      dev.characteristics[char_uuid_switch].readValue()
      .then((r) => {       
        const val = (r.getUint8(0)) ? 1 : 0;
        props.onLEDChange(val);
        console.log('got led state: ', val);
      })
      .catch((e) => {
        console.warn(e);
      });

      props.onConnect(dev);
    })
    .catch((e) => { console.warn(e) });
  }

  return (
    <div>
      <div>
        <button onClick={(e) => {
            if(typeof(device) === 'undefined'){
              addDevice();
            }else{
              removeDevice();
            }
          }}
        >
          {(typeof(device) === 'undefined') ? 'Connect' : 'Disconnect'}
        </button>
      </div>

      <div>
        {(typeof(device) === 'undefined') ? 'nothing connected' : 'device info'}
      </div>

      {
        (typeof(device) !== 'undefined') && 
        <div>
          <div>
            <div>{`device name: ${device.info.name}`}</div>
          </div>
          <div>
            <button
              onClick={async (e) => {
                console.log('I want to toggle the LED here!');

                var prev_val = 0;

                device.characteristics[char_uuid_switch].readValue()
                .then((r) => {       
                  const new_val = (r.getUint8(0)) ? 0 : 1;
                  const buffer = new ArrayBuffer(1);
                  const uint8 = new Uint8Array(buffer);
                  uint8.set([new_val], 0);
  
                  try {
                    device.characteristics[char_uuid_switch].writeValue(uint8);
                    props.onLEDChange(new_val);
                  }
                  catch (error) {
                    console.warn(error);
                  }
                  
                })
                .catch((e) => {
                  console.warn(e);
                });

              }}
            >
              Toggle LED
            </button>
          </div>
        </div>
      }
      
    </div>
  );
}



const MyResponsiveLine = ({ data /* see data tab */ }) => (
  <ResponsiveLine
      data={data}
      margin={{ top: 50, right: 110, bottom: 50, left: 60 }}
      xScale={{ type: 'linear', min: 'auto', max: 'auto' }}
      yScale={{ type: 'linear', min: 'auto', max: 'auto', stacked: false, reverse: false }}
      axisTop={null}
      axisRight={null}
      axisBottom={{
          orient: 'bottom',
          tickSize: 5,
          tickPadding: 5,
          tickRotation: 0,
          legend: 'time (ms)',
          legendOffset: 36,
          legendPosition: 'middle'
      }}
      axisLeft={{
          orient: 'left',
          tickSize: 5,
          tickPadding: 5,
          tickRotation: 0,
          legend: 'value',
          legendOffset: -40,
          legendPosition: 'middle'
      }}
      colors={{ scheme: 'nivo' }}
      pointSize={10}
      pointColor={{ theme: 'background' }}
      pointBorderWidth={2}
      pointBorderColor={{ from: 'serieColor' }}
      pointLabel="y"
      pointLabelYOffset={-12}
      useMesh={true}
      legends={[
          {
              anchor: 'bottom-right',
              direction: 'column',
              justify: false,
              translateX: 100,
              translateY: 0,
              itemsSpacing: 0,
              itemDirection: 'left-to-right',
              itemWidth: 80,
              itemHeight: 20,
              itemOpacity: 0.75,
              symbolSize: 12,
              symbolShape: 'circle',
              symbolBorderColor: 'rgba(0, 0, 0, .5)',
              effects: [
                  {
                      on: 'hover',
                      style: {
                          itemBackground: 'rgba(0, 0, 0, .03)',
                          itemOpacity: 1
                      }
                  }
              ]
          }
      ]}
  />
)






function App() {

  const now = new Date();
  const initialData = [ // these are in this order for the best color scheme! (x, y, z axes are well-contrasted)
    {id: 'led', data: []},
    {id: 'accx', data: []},
    {id: 'acct', data: []},
    {id: 'accy', data: []},
    {id: 'accz', data: []},
  ];

  const [start, setStart] = useState(now.getTime());
  const startRef = useRef(start);
  startRef.current = start;

  const [data, setData] = useState(initialData);
  const dataRef = useRef(data);
  dataRef.current = data;

  const [history, setHistory] = useState(10000); // ms of plot history
  const historyRef = useRef(history);
  historyRef.current = history;

  const [connected, setConnected] = useState(false);

  const enterData = (id, value) => {
    const timestamp = new Date();
    const entry = {y: value, x: String(timestamp.getTime() - startRef.current)};
    var newData = [...dataRef.current];
    newData.forEach((set, index) => {
      if(set.id === id){
        // console.log('updating: ', id, value);
        newData[index] = {...dataRef.current[index]};
        newData[index].data = [...newData[index].data, entry];
      }

      // enforce history length
      var old_record_count = 0;
      newData[index].data.forEach((entry, index) => {
        var limit = timestamp.getTime() - start - historyRef.current;
        if(limit < 0){
          limit = 0;
        }
        if(entry.x < limit){
          old_record_count += 1;
        }
      });
      newData[index].data.splice(0, old_record_count);
    })
    setData(newData);
  }

  return (
    <div className="App" style={{height: '100vh', width: '100vw'}}>
      
      <div style={{height: '20%', width: '100%'}}>
        <Bluetooth 
          onConnect={(dev) => {
            console.log('on connect', dev);

            // clear out old data and set start time
            setStart(now.getTime());
            setData(initialData);

            // add listeners and notifications for notable properties (such as ones that are changed by the device)
            dev.characteristics[char_uuid_acc_x].addEventListener('characteristicvaluechanged', (e) => {
              const x = e.target.value.getInt8(0);
              enterData('accx', x);
              // console.log('acc x changed to: ', x);
            });
            dev.characteristics[char_uuid_acc_x].startNotifications().then((e) => { console.log('started acc x notifications'); }).catch((e) => { console.warn(e); });

            dev.characteristics[char_uuid_acc_y].addEventListener('characteristicvaluechanged', (e) => {
              const y = e.target.value.getInt8(0);
              enterData('accy', y);
              // console.log('acc y changed to: ', y);
            });
            dev.characteristics[char_uuid_acc_y].startNotifications().then((e) => { console.log('started acc y notifications'); }).catch((e) => { console.warn(e); });

            dev.characteristics[char_uuid_acc_z].addEventListener('characteristicvaluechanged', (e) => {
              const z = e.target.value.getInt8(0);
              enterData('accz', z);
              // console.log('acc z changed to: ', z);
            });
            dev.characteristics[char_uuid_acc_z].startNotifications().then((e) => { console.log('started acc z notifications'); }).catch((e) => { console.warn(e); });

            dev.characteristics[char_uuid_acc_t].addEventListener('characteristicvaluechanged', (e) => {
              const t = e.target.value.getUint8(0);
              enterData('acct', t);
              // console.log('acc t changed to: ', t);
            });
            dev.characteristics[char_uuid_acc_t].startNotifications().then((e) => { console.log('started acc t notifications'); }).catch((e) => { console.warn(e); });

            // read the state of the led 
            const ledstate = (false) ? 1 : 0;
            enterData('led', ledstate);

            setConnected(true);
          }}
          onDisconnect={(e) => {
            console.log(e);
            setConnected(false);
          }}
          onLEDChange={(new_value) => {
            enterData('led', (new_value) ? 255 : 0);
          }}
        />
      </div>
      
      <div style={{height: '80%', width: '100%'}}>
        {connected && <MyResponsiveLine data={data} />}
      </div>
      
    </div>
  );
}

export default App;
