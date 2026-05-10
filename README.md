# ESP32 with 1.8" SPI 128*160 ILI9341 Screen

Very basic. All it does it make the screen display red for Landfill week or yellow for recycles week, between noon and 6pm local time, based on a Home Assistant topic, and be blank the rest of the time.

Home Assistant template for the sensor calculating rotating weeks from a known date:

```yaml
template:        
  - sensor:     
    - name: bin_day
      state: "{% set dt = strptime('Sep 26 2024', '%b %d %Y') %}
              {% set landiff = ((now().timestamp() / 86400) - (as_timestamp(dt) / 86400)) % 14 %}
              {% if landiff < 7 -%}Recycles{%- else -%}Landfill{%- endif %}"
    trigger:                     
     - platform: time_pattern
       minutes: "/30"
```

and I have mqtt_statestream set up to publish sensors, etc.