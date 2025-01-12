/**
 * Includes javascript code for index.html
 *
 * @author    Florian Staeblein
 * @date      2024/01/28
 * @copyright © 2024 Florian Staeblein
 */

// Global variables
var doughnutchart = null;
var websocket = null;
var clientID = 0;
var websocketConnected = false;
var lastAliveTimestamp = new Date(0);

(function()
{
  //IE9+ http://youmightnotneedjquery.com/
  function Ready(fn)
  {
    if (document.attachEvent ? document.readyState === "complete" : document.readyState !== "loading")
    {
      fn();
    }
    else
    {
      document.addEventListener('DOMContentLoaded', fn);
    }
  }
  
  // Call ready function
  Ready(SetupPage);

  function SetupPage() 
  {    
    // Initialize buttons with event handler
    [].forEach.call(document.getElementsByClassName('adjust-button'), function (adjustButton)
    {
      adjustButton.onclick = AdjustClick;
    });
    
    // Initialize checkbox
    var checkboxExpert = document.getElementById('ExpertSettings');
    checkboxExpert.onclick = OnToggleExpertSettings;
    checkboxExpert.checked = false;
    
    // Initialize slider for cycle timespan
    var sliderCycleTimespan = document.getElementById('sliderCycleTimespan');
    sliderCycleTimespan.oninput = OnInputCycleTimespan;
    sliderCycleTimespan.onchange = OnChangeCycleTimespan;
    sliderCycleTimespan.min = 200;
    sliderCycleTimespan.max = 1000;
    sliderCycleTimespan.step = 20;
    sliderCycleTimespan.value = 500;
        
    // Set default data (angles in 0-360°), size and event handlers in doughnut chart
    var setup = 
    {
      canvas: document.getElementById('doughnutchart'),
      data: [
        { label: '', color: "#969696", angle: 0   },
        { label: '', color: "#D5D5D5", angle: 135 },
        { label: '', color: "#494949", angle: 180 }],
      innerRadius: 0.5,
      outerRadius: 0.9,
      minAngle: 6.0,
      onchange: OnDoughnutChartChange,
      onshift: OnDoughnutChartShift
    };
    
    // Create doughnut chart
    doughnutchart = new DraggableDoughnutchart();
    doughnutchart.Setup(setup);
        
    // Start alive timer
    setInterval(CheckAlive, 500);
    
    // Start web socket and web events
    StartSocket();
    StartEvents();
  }

  // Starts the websocket
  function StartSocket()
  {
    websocket = new WebSocket('ws://' + document.location.host + '/websocket', ['arduino']);
    websocket.binaryType = "arraybuffer";
    
    // Connected handler
    websocket.onopen = function(e)
    {
      console.log("Websocket connected");
      websocketConnected = true;
      lastAliveTimestamp = Date.now();
    };
    
    // Disconnected handler
    websocket.onclose = function(e)
    {
      console.log("Websocket disconnected");
      websocketConnected = false;
    };
    
    // Error handler
    websocket.onerror = function(e)
    {
      console.log("Websocket error", e);
      websocketConnected = false;
    };
    
    // Message handler
    websocket.onmessage = function(e)
    {
      console.log("Websocket new message:" + e.data);
      lastAliveTimestamp = Date.now();
      
      if (e.data.startsWith("CLIENT_ID:"))
      {
        // Split the message by a pre-defined delimiter
        var delimiter = e.data.indexOf(":");
        var clientID_String = e.data.substring(delimiter + 1, e.data.length);
        
        var value_int = parseInt(clientID_String);
        
        if (value_int == NaN)
        {
          console.log("Data for client ID not matching (NaN is not allowed)");
          return;
        }
        
        if (value_int == 0)
        {
          console.log("Data for client ID not matching (0 is not allowed)");
          return;
        }
        
        clientID = value_int;
        
        console.log("Set [CLIENT_ID] = " + value_int);
      }
      else if (e.data.startsWith("MIXER_NAME:"))
      {
        // Split the message by a pre-defined delimiter
        var delimiter = e.data.indexOf(":");
        var name_String = e.data.substring(delimiter + 1, e.data.length);
            
        // Set new name
        var nameVar = document.getElementById('mixerName');
        var titleVar = document.getElementById('mixerTitle');
        nameVar.innerHTML = name_String;
        titleVar.innerHTML = name_String;
        
        console.log("Set [MIXER_NAME] = " + name_String);
        
        // Set image if available
        var imageVar = document.getElementById('mixerNameImage');
        imageVar.data = "logo_" + name_String.toLowerCase() + ".svg";
      }
      else if (e.data.startsWith("LIQUID_NAMES:") ||
        e.data.startsWith("LIQUID_COLORS:") ||
        e.data.startsWith("LIQUID_ANGLES:"))
      {
        // Split the message by a pre-defined delimiter
        var delimiter_0 = e.data.indexOf(":");
        var delimiter_1 = e.data.indexOf(",", delimiter_0 + 1);
        var delimiter_2 = e.data.indexOf(",", delimiter_1 + 1);

        var value1_String = e.data.substring(delimiter_0 + 1, delimiter_1);
        var value2_String = e.data.substring(delimiter_1 + 1, delimiter_2);
        var value3_String = e.data.substring(delimiter_2 + 1, e.data.length);
        
        if (!doughnutchart)
        {
          console.log("Doughnutchart is null");
          return;
        }
        
        if (e.data.startsWith("LIQUID_NAMES:"))
        {
          // Set new names
          var names = [ value1_String, value2_String, value3_String ];
          doughnutchart.Setnames(names);
      
          console.log("Set [LIQUID_NAMES] = " + names);
        }
        else if (e.data.startsWith("LIQUID_COLORS:"))
        {
          var value1_int = parseInt(value1_String);
          var value2_int = parseInt(value2_String);
          var value3_int = parseInt(value3_String);
          
          if (value1_int == NaN || value2_int == NaN || value3_int == NaN)
          {
            console.log("Data for liquid colors not matching (NaN is not allowed)");
            return;
          }
          
          var color1_string = "#" + value1_int.toString(16).padStart(6, "0");
          var color2_string = "#" + value2_int.toString(16).padStart(6, "0");
          var color3_string = "#" + value3_int.toString(16).padStart(6, "0");
          
          // Set new colors
          var colors = [ color1_string, color2_string, color3_string ];
          doughnutchart.Setcolors(colors);
        
          console.log("Set [LIQUID_COLORS] = " + colors);
        }
        else if (e.data.startsWith("LIQUID_ANGLES:"))
        {
          var angle1 = parseInt(value1_String);
          var angle2 = parseInt(value2_String);
          var angle3 = parseInt(value3_String);
          
          if (angle1 == NaN || angle2 == NaN || angle3 == NaN)
          {
            console.log("Data for liquid angles not matching (NaN is not allowed)");
            return;
          }
          
          if (angle1 < 0 || angle1 > 360 || angle2 < 0 || angle2 > 360 || angle3 < 0 || angle3 > 360)
          {
            console.log("Data for liquid angles not matching (must be within 0° and 360°)");
            return;
          }
          
          // Set new angles
          var angles = [ angle1, angle2, angle3 ];
          doughnutchart.Setangles(angles);
        
          console.log("Set [LIQUID_ANGLES] = " + angles);
        }
      }
      else if (e.data.startsWith("CYCLE_TIMESPAN:"))
      {
        // Split the message by a pre-defined delimiter
        var delimiter = e.data.indexOf(":");
        var cycletimespan_String = e.data.substring(delimiter + 1, e.data.length);
        
        var value_int = parseInt(cycletimespan_String);
        
        if (value_int == NaN)
        {
          console.log("Data for cycle timespan not matching (NaN is not allowed)");
          return;
        }
        
        if (value_int < 200 || value_int > 1000)
        {
          console.log("Data for cycle timespan not matching (must be within 200ms and 1000ms)");
          return;
        }
        
        // Set new cycle timespan value
        var output = document.getElementById('valueCycleTimespan');
        var slider = document.getElementById("sliderCycleTimespan");
        
        slider.value = value_int;
        output.innerHTML = value_int + "ms";
    
        console.log("Set [CYCLE_TIMESPAN] = " + value_int + "ms");
      }
    };
  }
  
  // Starts the events
  function StartEvents()
  {
    var events = new EventSource('/events');
    events.onopen = function(e)
    {
      console.log("Events Opened");
      lastAliveTimestamp = Date.now();
    };
    
    events.onerror = function(e)
    {
      if (e.target.readyState !== EventSource.OPEN)
      {
        console.log("Events Closed");
      }
    };
    
    events.onmessage = function(e)
    {
      console.log("Event: " + e.data);
      lastAliveTimestamp = Date.now();
    };
    
    events.addEventListener('ALIVE', function(e)
    {
      lastAliveTimestamp = Date.now();
      console.log("Set [ALIVE] = true");
      
    }, false);
    
    events.addEventListener('LIQUID_ANGLES', function(e)
    {
      console.log("Event[LIQUID_ANGLES]:" + e.data);
      lastAliveTimestamp = Date.now();
      
      // Split the message by a pre-defined delimiter
      var delimiter = e.data.indexOf(":");
      var clientID_String = e.data.substring(0, delimiter);
      var angles_String = e.data.substring(delimiter + 1, e.data.length);
      
      var clientID_int = parseInt(clientID_String);
      
      if (clientID_int == NaN)
      {
        console.log("Data for client ID not matching (NaN is not allowed)");
        return;
      }
      
      // Split angle data
      const angles = angles_String.split(",", 3);
      
      // Check for correct length
      if (angles.length !== 3)
      {
        console.log("Data length for liquid angles not matching (must be 3)");
        return;
      }
      
      // Get liquid angle values
      var angle1_int = parseInt(angles[0]);
      var angle2_int = parseInt(angles[1]);
      var angle3_int = parseInt(angles[2]);
      
      if (angle1_int < 0 || angle1_int > 360 || angle2_int < 0 || angle2_int > 360 || angle3_int < 0 || angle3_int > 360)
      {
        console.log("Data for liquid angles not matching (must be within 0° and 360°)");
        return;
      }
      
      // Check for own client ID
      if (clientID_int == clientID)
      {
        console.log("NOT Set [LIQUID_ANGLES] = " + angle1_int + "," + angle2_int + "," + angle3_int + " (Own Client ID)");
        return;
      }
      
      if (!doughnutchart)
      {
        console.log("Doughnutchart is null");
        return;
      }
      
      doughnutchart.Setangles([angle1_int, angle2_int, angle3_int]);
      
      console.log("Set [LIQUID_ANGLES] = " + angle1_int + "," + angle2_int + "," + angle3_int);
      
    }, false);
    
    events.addEventListener('CYCLE_TIMESPAN', function(e)
    {
      console.log("Event[CYCLE_TIMESPAN]:" + e.data);
      lastAliveTimestamp = Date.now();
      
      // Split the message by a pre-defined delimiter
      var delimiter = e.data.indexOf(":");
      var clientID_String = e.data.substring(0, delimiter);
      var cycletimespan_String = e.data.substring(delimiter + 1, e.data.length);
      
      var clientID_int = parseInt(clientID_String);
      var cycletimespan_int = parseInt(cycletimespan_String);
      
      if (cycletimespan_int == NaN || clientID_int == NaN)
      {
        console.log("Data for cycle timespan or client ID not matching (NaN is not allowed)");
        return;
      }
      
      // Check for correct data range
      if (cycletimespan_int < 200 || cycletimespan_int > 1000)
      {
        console.log("Data for cycle timespan not matching (must be within 200ms and 1000ms)");
        return;
      }
      
      // Check for own client ID
      if (clientID_int == clientID)
      {
        console.log("NOT Set [CYCLE_TIMESPAN] = " + cycletimespan_int + "ms (Own Client ID)");
        return;
      }
      
      // Set new cycle timespan value
      var output = document.getElementById('valueCycleTimespan');
      var slider = document.getElementById("sliderCycleTimespan");
      
      slider.value = cycletimespan_int;
      output.innerHTML = cycletimespan_int + "ms";
      
      console.log("Set [CYCLE_TIMESPAN] = " + cycletimespan_int + "ms");
    
    }, false);
  }
  
  // Function checks every 500ms if the communication is online. Timeout is 1.5s
  function CheckAlive()
  {
    if (doughnutchart)
    {
      doughnutchart.Setonline(Date.now() - lastAliveTimestamp < 1500);
    }
  }
  
  // Will be called if a value of the doughnutchart has changed
  function OnDoughnutChartChange(fromUserInput, currentIndex)
  {    
    for (var index = 0; index < doughnutchart.data.length; index++)
    {      
      // Get changed doughnut chart value
      var percentage = doughnutchart.GetSliceSizePercentage(index);
   
      // Get table cells
      var headerCell = document.getElementById("labelLiquid" + index);
      var valueCell = document.getElementById("varLiquid" + index );
      
      // Set header cell
      headerCell.style.backgroundColor = doughnutchart.data[index].color;
      headerCell.innerHTML = doughnutchart.data[index].label;
      
      // Set value cell
      // Values smaller than minAngle are per default 0%
      valueCell.innerHTML = (percentage <= (doughnutchart.minAngle / 360.0 * 100.0) ? 0 : percentage).toFixed(0) + "%";
    };
  }
  
  // Will be called if an angle of the doughnutchart has shifted. increments is signed and in degrees
  function OnDoughnutChartShift(index, increments)
  {
    // Build websocket message
    var websocketMessage = "LIQUID_INCREMENT:" + index + "," + increments.toFixed(0);
  
    // Send websocket
    if (websocketConnected)
    {
      websocket.send(websocketMessage);
      console.log("Websocket send:" + websocketMessage + " -> success");
    }
    else
    {
      console.log("Websocket send:" + websocketMessage + " -> no websocket..");
      if (confirm("The control is not connected. Reload page?"))
      {
        window.location.reload();
      }
    }
  }

  // Toggle visibillity on checked changed
  function OnToggleExpertSettings()
  {
    // Get table and expert settings check box
    var expertSettingsTable = document.getElementById('expertSettings-table');
    var expertSettingsCheckbox = document.getElementById('ExpertSettings');

    // Change visibility
    expertSettingsTable.style.display = expertSettingsCheckbox.checked == true ? 'block' : 'none';
  }

  // Will be called if plus or minus button is clicked
  function AdjustClick()
  {
    var segmentIndex = parseInt(this.getAttribute('data-i'));
    var distance = parseInt(this.getAttribute('data-d'));

    // Move angle by 3 degrees further
    if (doughnutchart)
    {
      doughnutchart.MoveAngle(segmentIndex, distance * 3);
    }
  }

  // Will be called if new slider value is present
  function OnInputCycleTimespan()
  {
    var output = document.getElementById('valueCycleTimespan');
    var slider = document.getElementById("sliderCycleTimespan");
    
    output.innerHTML = slider.value + "ms";
    
    // Build websocket message
    var websocketMessage = "CYCLE_TIMESPAN:" + slider.value;
    
    // Send websocket
    if (websocketConnected)
    {
      websocket.send(websocketMessage);
      console.log("Websocket send:" + websocketMessage + "ms -> success");
    }
    else
    {
      console.log("Websocket send:" + websocketMessage + "ms -> no websocket..");
      if (confirm("The control is not connected. Reload page?"))
      {
        window.location.reload();
      }
    }
  }

  // Will be called if new slider value is changed
  function OnChangeCycleTimespan()
  {
    if (websocketConnected)
    {
      websocket.send("SAVE");
      console.log("Websocket send:SAVE");
    }
    else
    {
      console.log("SAVE: no websocket..");
      if (confirm("The control is not connected. Reload page?"))
      {
        window.location.reload();
      }
    }
  }

})();

