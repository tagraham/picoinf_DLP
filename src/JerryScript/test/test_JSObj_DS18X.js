let sensor = new DS18X(21)

Log(`sensor.IsAlive()                  = ${sensor.IsAlive()}`)
sensor.SetResolution(9)
Log(`sensor.GetTemperatureCelsius()    = ${sensor.GetTemperatureCelsius()}`)
Log(`sensor.GetTemperatureFahrenheit() = ${sensor.GetTemperatureFahrenheit()}`)