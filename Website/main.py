from flask import Flask, render_template, redirect, url_for
from datetime import datetime
app = Flask(__name__)

batteryMax = 10000

@app.route('/')
def home():
    fp = open('static/data/battery_level.txt', 'r')
    batteryWh = fp.readline()
    batteryLevel = ((int(batteryWh.replace('\n', '')) / batteryMax) * 100).__trunc__()
    solarW = "%.2f" % float(fp.readline())
    ecosystemW = "%.2f" % float(fp.readline())
    batteryW = "%.2f" % (float(solarW.replace('\n', '')) - float(ecosystemW.replace('\n', '')))
    fp.close()
    lastRefreshedDateTime = datetime.now().strftime("%d/%m/%Y %H:%M:%S %p")
    batteryColor = 'bg-gradient-ocean' if float(batteryW) > 0.0 else 'bg-gradient-sunset ' if float(batteryW) < 0.0 else 'bg-gradient-graphite'
    solarColor = 'bg-gradient-ocean' if float(solarW) > 0.0 else 'bg-gradient-sunset' if float(solarW) < 0.0 else 'bg-gradient-graphite'
    ecosystemColor = 'bg-gradient-ocean' if float(ecosystemW) > 0.0 else 'bg-gradient-sunset' if float(ecosystemW) < 0.0 else 'bg-gradient-graphite'
    return render_template("index.html", lastRefreshedDateTime=lastRefreshedDateTime, batteryWh=batteryWh,
                           batteryLevel=batteryLevel, solarW=solarW,
                           ecosystemW=ecosystemW, batteryW=batteryW, batteryStatus='CHARGING',
                           batteryColor=batteryColor, solarColor=solarColor, ecosystemColor=ecosystemColor)


if __name__ == '__main__':
    app.run(debug=True)
