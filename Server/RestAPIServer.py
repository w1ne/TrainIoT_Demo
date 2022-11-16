# -----------------------------------------------------------------------------
# Importing the modules
# -----------------------------------------------------------------------------
import paho.mqtt.client as mqtt
from flask import Flask
app = Flask(__name__)

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    mqttc.subscribe("loco/location/lat")
    mqttc.subscribe("loco/location/lng")
    mqttc.subscribe("loco/power")

def on_message_location_lat(client, userdata, message):
   global current_lat 
   current_lat = message.payload.decode()

def on_message_location_lng(client, userdata, message):
   global current_lng 
   current_lng = message.payload.decode()

def on_message_location_power(client, userdata, message):
   global current_power 
   current_power = message.payload.decode()

mqttc = mqtt.Client()
#mqttc.tls_set(ca_certs="ca.crt")
#mqttc.tls_insecure_set(True)
mqttc.connect("49.12.32.132", 1883, 60) #49.12.32.132

mqttc.message_callback_add("loco/location/lat", on_message_location_lat)
mqttc.message_callback_add("loco/location/lng", on_message_location_lng)
mqttc.message_callback_add("loco/power", on_message_location_power)
mqttc.on_connect = on_connect

mqttc.loop_start()

@app.route('/')
def index():
  return 'Index Page'

@app.route('/loco/motor/PWM/<int:post_id>', methods=['POST'])
def mqtt_motorpwm(post_id):
  mqttc.publish("loco/control/motor/power", post_id)
  return str(post_id)

@app.route('/loco/motor/state/<int:post_id>', methods=['POST'])
def mqtt_motorstate(post_id):
  if post_id >= 1:
    qttc.publish("loco/control/motor/switch","on")
  else: 
    qttc.publish("loco/control/motor/switch","off")
  return str(post_id)

# -----------------------------------------------------------------------------
# Main function
# -----------------------------------------------------------------------------
if __name__ == "__main__":
    app.run(debug=True, 
         host='0.0.0.0', 
         port=9000, 
         threaded=True)

#print(app.url_map)