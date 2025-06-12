# MQTT ↔ Serial Bridge for Ulanzi TC001

This Python app bridges MQTT messages and serial commands for the Ulanzi TC001 LED matrix display.

## Features

- Sends display commands over serial to the device (scrolling text, images, colors).
- Subscribes to MQTT topics to receive messages and images.
- Publishes button presses from the device back to MQTT.
- Supports whitelisted image files (converted on the fly).

## Requirements

- Python 3.8+
- MQTT broker (e.g., Mosquitto)
- Serial access to device (e.g., `/dev/ttyUSB0`)

## Installation

Install dependencies:

```bash
pip install -r requirements.txt
````

## Usage

```bash
python mqtt2serial.py SERIAL_PORT MQTT_PREFIX [--mqtt_host HOST] [--mqtt_port PORT]
```

### Example

```bash
python mqtt2serial.py /dev/ttyUSB0 clock --mqtt_host localhost --mqtt_port 1881
```

## MQTT Topics

| Topic            | Direction | Description                                |
| ---------------- | --------- | ------------------------------------------ |
| `prefix/color`   | → device  | Set color (`r,g,b`)                        |
| `prefix/message` | → device  | Scroll message text                        |
| `prefix/static`  | → device  | Show text (not scrolling)                  |
| `prefix/image`   | → device  | Send whitelisted image name                |
| `prefix/buttons` | ← device  | Publishes button presses (e.g. `BTN_LEFT`) |

## Image Requirements

Whitelisted image files must be:

* Located in the `images/` directory
* Named with `.png`
* 32×8 or will be resized

## Docker

### Build

```bash
docker build -t mqtt2serial .
```

### Run

```bash
docker run --rm --device=/dev/ttyUSB0 mqtt2serial \
  /dev/ttyUSB0 clock --mqtt_host 192.168.1.10 --mqtt_port 1881
```

## Deploy on edge (for the tutorial)

### Build

```
podman build -t isc-tutorial.hlrs.de/mqtt2serial:1 .
podman push mqtt2serial
```

### Deploy

```
export CLOCK_NAME=clock1
export CLOCK_EDGE_MQTT=$(kubectl get nodes -l display-name=$CLOCK_NAME -o jsonpath='{.items[0].status.addresses[?(@.type=="InternalIP")].address}')

envsubst < deployment.yaml | kubectl apply -f -
```
### Remove

```
export CLOCK_NAME=clock1
export CLOCK_EDGE_MQTT=$(kubectl get nodes -l display-name=$CLOCK_NAME -o jsonpath='{.items[0].status.addresses[?(@.type=="InternalIP")].address}')

envsubst < deployment.yaml | kubectl delete -f -
```


## License

MIT
