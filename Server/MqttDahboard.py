# -----------------------------------------------------------------------------
# Importing the modules
# -----------------------------------------------------------------------------
import dash
from dash import html, dcc, Input, Output, ctx
import dash_bootstrap_components as dbc
import paho.mqtt.client as mqtt
import plotly.graph_objects as go
import plotly.express as px
import plotly
import pandas as pd
from collections import deque
import random


df = pd.DataFrame({'Loco': ["EMD G38"],
                   'Power': [0.0],
                   'Voltage': [0.0],
                   'Lat': [0.0],
                   'Lon': [0.0]})

Voltage_X = deque(maxlen = 20)
Voltage_X.append(0)
Voltage_Y = deque(maxlen = 20)
Voltage_Y.append(0)
Current_X = deque(maxlen = 20)
Current_X.append(0)
Current_Y = deque(maxlen = 20)
Current_Y.append(0)

PressureOne_X = deque(maxlen = 20)
PressureOne_X.append(0)
PressureOne_Y = deque(maxlen = 20)
PressureOne_Y.append(0)
PressureTwo_X = deque(maxlen = 20)
PressureTwo_X.append(0)
PressureTwo_Y = deque(maxlen = 20)
PressureTwo_Y.append(0)


global current_lng
global current_lat
global current_power
global current_voltage
global current_amp
global current_pressure_one
global current_pressure_two

global engine_state
global brake_state

current_lng = "NaN"
current_lat = "NaN"
current_power = "NaN"
current_voltage = "NaN"
current_amp = "NaN"
current_pressure_one = "4.8"
current_pressure_two = "4.8"

engine_state = False
brake_state = False

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    mqttc.subscribe("loco/location/lat")
    mqttc.subscribe("loco/location/lng")
    mqttc.subscribe("loco/power")
    mqttc.subscribe("loco/voltage")
    mqttc.subscribe("loco/current")
    mqttc.subscribe("loco/brake/pressure/1/")
    mqttc.subscribe("loco/brake/pressure/2/")

def on_message_location_lat(client, userdata, message):
   global current_lat 
   current_lat = message.payload.decode()

def on_message_location_lng(client, userdata, message):
   global current_lng 
   current_lng = message.payload.decode()

def on_message_power(client, userdata, message):
   global current_power 
   current_power = message.payload.decode()

def on_message_current(client, userdata, message):
   global current_amp
   current_amp = message.payload.decode()

def on_message_voltage(client, userdata, message):
   global current_voltage
   current_voltage = message.payload.decode()

def on_message_pressure_one(client, userdata, message):
   global current_pressure_one
   #current_pressure_one = message.payload.decode()

def on_message_pressure_two(client, userdata, message):
   global current_pressure_two
   #current_pressure_two = message.payload.decode()

mqttc = mqtt.Client()
#mqttc.tls_set(ca_certs="ca.crt")
#mqttc.tls_insecure_set(True)
mqttc.connect("49.12.32.132", 1883, 60) #49.12.32.132

mqttc.message_callback_add("loco/location/lat", on_message_location_lat)
mqttc.message_callback_add("loco/location/lng", on_message_location_lng)
mqttc.message_callback_add("loco/power", on_message_power)
mqttc.message_callback_add("loco/voltage", on_message_voltage)
mqttc.message_callback_add("loco/current", on_message_current)

mqttc.message_callback_add("loco/brake/pressure/1", on_message_pressure_one)
mqttc.message_callback_add("loco/brake/pressure/2", on_message_pressure_two)

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
        #dbc.Row(dbc.Col(card_lat, lg=4)),
        #dbc.Row(dbc.Col(card_lng, lg=4)),
        #dbc.Row(dbc.Col(card_power, lg=4)),
        html.H2("Voltage/Current/Power"),
        html.Div(
            children=[
                dcc.Graph(id="voltage_graph", style={'display': 'inline-block', "width":400, "margin": 0,}),
                dcc.Graph(id="current_graph", style={'display': 'inline-block', "width":400, "margin": 0,}),
                dcc.Graph(id='gauge', figure=figmap, style={'display': 'inline-block', "width":400, "margin": 0,})
            ]
        ),
        html.Div([
        html.Button('Engine', id='btn-nclicks-engine', n_clicks=0),
        html.Button('Brake', id='btn-nclicks-brake', n_clicks=0),
        html.Button('Engine 100%', id='btn-nclicks-emax', n_clicks=0),
        html.Button('Engine 75%', id='btn-nclicks-ehalf', n_clicks=0),
        html.Div(id='container-button-timestamp')]),
        dcc.Interval(id = 'graph-update', interval = 5000, n_intervals = 0),
        html.Div(
            children=[
                dcc.Graph(id="pressure_one_graph", style={'display': 'inline-block', "width":400, "margin": 0,}),
                dcc.Graph(id="pressure_two_graph", style={'display': 'inline-block', "width":400, "margin": 0,}),
            ]
        ),
        html.Hr(),
        dcc.Graph(
        id='map',
        figure=figpower),
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

@app.callback(
    Output('voltage_graph', 'figure'),
    [ Input('graph-update', 'n_intervals') ]
)
def update_graph_voltage(n):
    Voltage_X.append(Voltage_X[-1]+5)
    Voltage_Y.append(float(current_voltage))
  
    data = plotly.graph_objs.Scatter(
            x=list(Voltage_X),
            y=list(Voltage_Y),
            name='Scatter',
            mode= 'lines+markers'
    )
  
    return {'data': [data],
            'layout' : go.Layout(xaxis = dict(range=[min(Voltage_X),max(Voltage_X)]),
                                 yaxis = dict(range = [min(Voltage_Y),max(Voltage_Y)]), title = 'Voltage[mA])')}
 
@app.callback(
    Output('current_graph', 'figure'),
    [ Input('graph-update', 'n_intervals') ]
)
def update_graph_current(n):
    Current_X.append(Current_X[-1]+5)
    Current_Y.append(float(current_amp))
  
    data = plotly.graph_objs.Scatter(
            x=list(Current_X),
            y=list(Current_Y),
            name='Scatter',
            mode= 'lines+markers'
    )
  
    return {'data': [data],
            'layout' : go.Layout(xaxis=dict(range=[min(Current_X),max(Current_X)]),
                                 yaxis = dict(range = [min(Current_Y),max(Current_Y)]),title = 'Current[mA]' )}

@app.callback(
    Output('pressure_one_graph', 'figure'),
    [ Input('graph-update', 'n_intervals') ]
)
def update_graph_current(n):
    PressureOne_X.append(PressureOne_X[-1]+5)
    PressureOne_Y.append(float(current_pressure_one))
  
    data = plotly.graph_objs.Scatter(
            x=list(PressureOne_X),
            y=list(PressureOne_Y),
            name='Scatter',
            mode= 'lines+markers'
    )
  
    return {'data': [data],
            'layout' : go.Layout(xaxis = dict(range=[min(PressureOne_X),max(PressureOne_X)]),
                                 yaxis = dict(range = [min(PressureOne_Y),max(PressureOne_Y)]),title = 'Pressure Node 1[mBar]' )}

@app.callback(
    Output('pressure_two_graph', 'figure'),
    [ Input('graph-update', 'n_intervals') ]
)
def update_graph_current(n):
    PressureTwo_X.append(PressureTwo_X[-1]+5)
    PressureTwo_Y.append(float(current_pressure_two))
  
    data = plotly.graph_objs.Scatter(
            x=list(PressureTwo_X),
            y=list(PressureTwo_Y),
            name='Scatter',
            mode= 'lines+markers'
    )
  
    return {'data': [data],
            'layout' : go.Layout(xaxis=dict(range=[min(PressureTwo_X),max(PressureTwo_X)]),
                                 yaxis = dict(range = [min(PressureTwo_Y),max(PressureTwo_Y)]),title = 'Pressure Node 2[mBar]' )}

# Update map graph
@app.callback(
    Output("map", "figure"),
    Input('update', 'n_intervals'),
)
def update_map(timer):
    df.at[0, 'Lat'] = current_lat
    df.at[0, 'Lon'] = current_lng
    #df.at[0, 'Power'] = current_power
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
    Input('btn-nclicks-engine', 'n_clicks'),
    Input('btn-nclicks-brake', 'n_clicks'),
    Input('btn-nclicks-emax', 'n_clicks'),
    Input('btn-nclicks-ehalf', 'n_clicks')
)
def displayClick(btn1, btn2, btn3, btn4):
    msg = ""
    if "btn-nclicks-engine" == ctx.triggered_id:
        global engine_state
        if (engine_state == False):
            msg = "Engine ON"
            mqttc.publish("loco/control/motor/switch","on")
            engine_state = True
        else:
            msg = "Engine OFF"
            mqttc.publish("loco/control/motor/switch", "off")
            engine_state = False
    if "btn-nclicks-brake" == ctx.triggered_id:
        global brake_state
        if (brake_state == False):
            msg = "Brake ON"
            mqttc.publish("loco/control/brake/switch","on")
            brake_state = True
            current_pressure_one = "5.0"
            current_pressure_two = "5.0"
        else:
            msg = "Brake OFF"
            mqttc.publish("loco/control/brake/switch", "off")
            brake_state = False
            current_pressure_one = "4.8"
            current_pressure_two = "4.8"
    elif "btn-nclicks-emax" == ctx.triggered_id:
        msg = "Engine MAX"
        mqttc.publish("loco/control/motor/power", 255)
    elif "btn-nclicks-ehalf" == ctx.triggered_id:
        msg = "Engine 75%"
        mqttc.publish("loco/control/motor/power", 150)
    return html.Div(msg)
# -----------------------------------------------------------------------------
# Main function
# -----------------------------------------------------------------------------
if __name__ == "__main__":
    app.run_server(debug=True, host= '0.0.0.0')