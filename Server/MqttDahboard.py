# -----------------------------------------------------------------------------
# Importing the modules
# -----------------------------------------------------------------------------
import dash
from dash import html, dcc, Input, Output, ctx
import dash_bootstrap_components as dbc
import paho.mqtt.client as mqtt
import plotly.graph_objects as go
import plotly.express as px
import pandas as pd


df = pd.DataFrame({'Loco': ["EMD G38"],
                   'Power': [0.0],
                   'Voltage': [0.0],
                   'Lat': [0.0],
                   'Lon': [0.0]})

global current_lng
global current_lat
global current_power

current_lng = "NaN"
current_lat = "NaN"
current_power = "NaN"

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

# -----------------------------------------------------------------------------
# Defining Dash app
# -----------------------------------------------------------------------------
app = dash.Dash(external_stylesheets=[dbc.themes.FLATLY])
# -----------------------------------------------------------------------------
# Location card
# -----------------------------------------------------------------------------
card_lng = dbc.Card(
    html.H4(id="Longtitude")
)

card_lat = dbc.Card(
    html.H4(id="Latitude")
)

card_power = dbc.Card(
    html.H4(id="Power")
)

figpower = go.Figure()
figpower.add_trace(go.Indicator(
    name = "Power",
    domain={'x': [0, 1], 'y': [0, 1]},
    value=0,
    mode="gauge+number+delta",
    title={'text': "Power[mW]"},
    delta={'reference': 380},
    gauge={'axis': {'range': [None, 4000]},
           'steps': [
               {'range': [0, 1500], 'color': "lightgray"},
               {'range': [1500, 3000], 'color': "gray"},
               {'range': [3000, 4000], 'color': "hotpink"}],
           'threshold': {'line': {'color': "red", 'width': 4}, 'thickness': 0.75, 'value': 4000}}))
default_lat = float(47.472057)
default_lan = float(19.287760)
figmap = px.scatter_mapbox(df, lat="Lat", lon="Lon", hover_name="Loco", center={"lat":default_lat, "lon":default_lan},
                        color_discrete_sequence=["fuchsia"], zoom=10, height=300)
figmap.update_layout(mapbox_style="open-street-map")
figmap.update_layout(margin={"r":0,"t":0,"l":0,"b":0})

# -----------------------------------------------------------------------------
# Application layout
# -----------------------------------------------------------------------------
app.layout = dbc.Container(
    [
        dcc.Interval(id='update', n_intervals=0, interval=1000*2),
        html.H1("IoT Loco Dashboard"),
        html.Hr(),
        dbc.Row(dbc.Col(card_lat, lg=4)),
        dbc.Row(dbc.Col(card_lng, lg=4)),
        dbc.Row(dbc.Col(card_power, lg=4)),
        dcc.Graph(
        id='map',
        figure=figpower),
        dcc.Graph(
        id='gauge',
        figure=figmap),
        html.Div([
        html.Button('Engine ON', id='btn-nclicks-eon', n_clicks=0),
        html.Button('Engine OFF', id='btn-nclicks-eof', n_clicks=0),
        html.Button('Engine 100%', id='btn-nclicks-emax', n_clicks=0),
        html.Button('Engine 75%', id='btn-nclicks-ehalf', n_clicks=0),
        html.Div(id='container-button-timestamp')])
    ]
)

# -----------------------------------------------------------------------------
# Callback for updating data
# -----------------------------------------------------------------------------
@app.callback(
    Output('Latitude', 'children'),
    Input('update', 'n_intervals'),
)
def update_lat(timer):
    return ("Latitude: " + (current_lat))

@app.callback(
    Output('Longtitude', 'children'),
    Input('update', 'n_intervals'),
)
def update_lng(timer):
    return ("Longtitude: " + (current_lng))

@app.callback(
    Output('Power', 'children'),
    Input('update', 'n_intervals'),
)
def update_power(timer):
    return ("Power: " + (current_power))

# Update map graph
@app.callback(
    Output("map", "figure"),
    Input('update', 'n_intervals'),
)
def update_map(timer):
    df.at[0, 'Lat'] = current_lat
    df.at[0, 'Lon'] = current_lng
    df.at[0, 'Power'] = current_power
    #print(df)
    figmap.update_traces(lat = [float(current_lat)], lon = [float(current_lng)])
    figmap.update_geos(fitbounds="locations")  
    return figmap

# Update gauge graph
@app.callback(
    Output("gauge", "figure"),
    Input('update', 'n_intervals'),
)
def update_gauge(timer):
    figpower.update_traces(value= float(current_power), selector=dict(name="Power"))
    return figpower

@app.callback(
    Output('container-button-timestamp', 'children'),
    Input('btn-nclicks-eon', 'n_clicks'),
    Input('btn-nclicks-eof', 'n_clicks'),
    Input('btn-nclicks-emax', 'n_clicks'),
    Input('btn-nclicks-ehalf', 'n_clicks')
)
def displayClick(btn1, btn2, btn3, btn4):
    msg = ""
    if "btn-nclicks-eon" == ctx.triggered_id:
        msg = "Engine ON"
        mqttc.publish("loco/control/motor/switch","on")
    elif "btn-nclicks-eof" == ctx.triggered_id:
        msg = "Engine OFF"
        mqttc.publish("loco/control/motor/switch", "off")
    elif "btn-nclicks-emax" == ctx.triggered_id:
        msg = "Engine MAX"
        mqttc.publish("loco/control/motor/power", 255)
    elif "btn-nclicks-ehalf" == ctx.triggered_id:
        msg = "Engi ne 75%"
        mqttc.publish("loco/control/motor/power", 150)
    return html.Div(msg)
# -----------------------------------------------------------------------------
# Main function
# -----------------------------------------------------------------------------
if __name__ == "__main__":
    app.run_server(debug=True)