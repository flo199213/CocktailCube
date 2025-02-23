/**
 * Includes javascript code for index.html
 *
 * @author    Florian Staeblein
 * @date      2025/02/21
 * @copyright © 2025 Florian Staeblein
 */

// Global variables
var doughnutchart = null;
var needUpdateCounter = -1;
var isMixer = true;
var lastAliveTimestamp = new Date(0);
var isSliding = false;

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

  async function SetupPage() 
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
    sliderCycleTimespan.onmousedown = () => { isSliding = true; };
    sliderCycleTimespan.ontouchstart = () => { isSliding = true; };
    sliderCycleTimespan.min = 200;
    sliderCycleTimespan.max = 1000;
    sliderCycleTimespan.step = 20;
    sliderCycleTimespan.value = 500;
    
    // Detect mouse or touch release
    document.addEventListener('mouseup', () => { isSliding = false; });
    document.addEventListener('touchend', () => { isSliding = false; });
        
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
      onshift: OnDoughnutChartShift,
      onshifted: OnDoughnutChartShifted
    };
    
    // Create doughnut chart
    doughnutchart = new DraggableDoughnutchart();
    doughnutchart.Setup(setup);
    
    // First alive check
    CheckAlive();
    
    // Start alive timer
    setInterval(CheckAlive, 500);    
  }
  
  async function FetchMixerSettings()
  {
    try
    {
      var response = await fetch('http://' + document.location.host + '/control?settings=0');
      
      // Check for response
      if (!response.ok)
      {
        return;
      }
      
      // Get Json data
      var data = await response.json();
      var mixer = data[0];
      
      // Set alive timestamp
      lastAliveTimestamp = Date.now();
     
      // Parse settings
      await ParseMixerSettings(mixer);
    }
    catch (error)
    {
      console.error('Error getting mixer settings:', error);
    }
  }
  
  async function FetchMixerValues()
  {
    var newNeedUpdateCounter = -1;
    
    try
    {
      var response = await fetch('http://' + document.location.host + '/control?values=0');
      
      // Check for response
      if (!response.ok)
      {
        return;
      }
      
      // Get Json data
      var data = await response.json();
      var mixer = data[0];
      
      // Set alive timestamp
      lastAliveTimestamp = Date.now();
      
      // Parse values
      newNeedUpdateCounter = await ParseMixerValues(mixer);
    }
    catch (error)
    {
      console.error('Error getting mixer values:', error);
    }
    
    return newNeedUpdateCounter;
  }

  // Will parse the json document 'mixer' for new settings
  async function ParseMixerSettings(mixer)
  {
    try
    {
      // IS_MIXER:
      
      // Set is mixer
      isMixer = mixer.IS_MIXER;
      console.log("Set [IS_MIXER] = " + mixer.IS_MIXER);
      
      // MIXER_NAME:
      
      // Set new name
      document.getElementById('mixerName').innerHTML = mixer.MIXER_NAME;
      document.getElementById('mixerTitle').innerHTML = mixer.MIXER_NAME;
      console.log("Set [MIXER_NAME] = " + mixer.MIXER_NAME);
      
      // Set image if available
      var imageVar = document.getElementById('mixerNameImage');
      imageVar.data = "logo_" + mixer.MIXER_NAME.toLowerCase() + ".svg";
      
      if (doughnutchart)
      {
        // LIQUID_NAME_X..:
        
        // Set new names
        var names = [ mixer.LIQUID_NAME_1, mixer.LIQUID_NAME_2, mixer.LIQUID_NAME_3 ];
        doughnutchart.Setnames(names);
        console.log("Set [LIQUID_NAME_X..] = " + names);
        
        // LIQUID_COLOR_X..:
        
        // Set new colors
        var colors = [ mixer.LIQUID_COLOR_1, mixer.LIQUID_COLOR_2, mixer.LIQUID_COLOR_3 ];
        doughnutchart.Setcolors(colors);
        console.log("Set [LIQUID_COLOR_X..] = " + colors);
      }
      else
      {
        console.log("Doughnutchart is null");
      }      
    }
    catch (error)
    {
      console.error('Error parsing mixer settings:', error);
    }
  }
  
  // Will parse the json document 'mixer' for new values
  async function ParseMixerValues(mixer)
  {
    var newNeedUpdateCounter = -1;
    
    try
    {
      // NEED_UPDATE:
      newNeedUpdateCounter = mixer.NEED_UPDATE;
      console.log("Got new [NEED_UPDATE] = " + mixer.NEED_UPDATE);
            
      if (doughnutchart)
      {
        // LIQUID_ANGLE_X..:
        
        if (isMixer &&
          mixer.LIQUID_ANGLE_1 != NaN &&
          mixer.LIQUID_ANGLE_1 >= 0 &&
          mixer.LIQUID_ANGLE_1 <= 360 &&
          mixer.LIQUID_ANGLE_2 != NaN &&
          mixer.LIQUID_ANGLE_2 >= 0 &&
          mixer.LIQUID_ANGLE_2 <= 360 &&
          mixer.LIQUID_ANGLE_3 != NaN &&
          mixer.LIQUID_ANGLE_3 >= 0 &&
          mixer.LIQUID_ANGLE_3 <= 360)
        {
          // Set new angles
          var angles = [ mixer.LIQUID_ANGLE_1, mixer.LIQUID_ANGLE_2, mixer.LIQUID_ANGLE_3 ];
          doughnutchart.Setangles(angles);
          console.log("Set [LIQUID_ANGLE_X..] = " + angles);
        }
        else if (!isMixer)
        {
          // Set default angles
          var angles = [ 0, 120, 240 ];
          doughnutchart.Setangles(angles);
          console.log("Not set [LIQUID_ANGLE_X..] -> No mixer");
        }
        else
        {
          console.log("Data for liquid angles not matching (NaN is not allowed and must be within 0° and 360°)");
        }
        
        // CYCLE_TIMESPAN:
        
        if (mixer.CYCLE_TIMESPAN != NaN &&
          mixer.CYCLE_TIMESPAN >= 200 &&
          mixer.CYCLE_TIMESPAN <= 1000)
        {
          // Avoid setting pwm cycle timespan while using slider
          if (!isSliding)
          {
            // Set new cycle timespan value
            var output = document.getElementById("valueCycleTimespan");
            var slider = document.getElementById("sliderCycleTimespan");
          
            output.innerHTML = mixer.CYCLE_TIMESPAN + "ms";
            slider.value = mixer.CYCLE_TIMESPAN;
            console.log("Set [CYCLE_TIMESPAN] = " + mixer.CYCLE_TIMESPAN + "ms");
          }
        }
        else
        {
          console.log("Data for cycle timespan not matching (NaN is not allowed and must be within 200ms and 1000ms)");
          return;
        }
      }
      else
      {
        console.log("Doughnutchart is null");
      }
    }
    catch (error)
    {
      console.error('Error parsing mixer values:', error);
    }
    
    return newNeedUpdateCounter;
  }
    
  // Function checks every 500ms if the communication is online. Timeout is 1.5s
  async function CheckAlive()
  {
    if (isSliding ||
      (doughnutchart && doughnutchart.IsDragged()))
    {
      return;
    }
    
    // Get current mixer values
    var newNeedUpdateCounter = await FetchMixerValues();
    
    // Check for new configuration
    if (newNeedUpdateCounter != -1 &&
      newNeedUpdateCounter != needUpdateCounter)
    {
      needUpdateCounter = newNeedUpdateCounter;
      await FetchMixerSettings();
    }
      
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
  
  // Will be called if an angle of the doughnutchart is shifting. Increments is signed and in degrees
  function OnDoughnutChartShift(index, increments)
  {
    console.log("OnDoughnutChartShift " + index + " - " + increments);
  }

  // Will be called if an angle of the doughnutchart has shifted. Increments is signed and in degrees
  async function OnDoughnutChartShifted(index, increments)
  {
    console.log("OnDoughnutChartShifted " + index + " - " + increments);
    
    if (!isMixer)
    {
      alert("The mixer is in bar mode. Control not available")
      return;
    }
    
    try
    {    
      var response = await fetch('http://' + document.location.host + '/control?LIQUID_ANGLE_' + (index + 1) + '=' + increments.toFixed(0),
      {
        method: 'PUT'
      });
      
      // Check for response
      if (!response.ok)
      {
        console.log("Send: LIQUID_ANGLE_" + index + " no connection..");
        if (confirm("The control is not connected. Reload page?"))
        {
          window.location.reload();
        }
        return;
      }
      
      console.log("Send: LIQUID_ANGLE_" + index + " successful");
            
      // Set alive timestamp
      lastAliveTimestamp = Date.now();
    }
    catch (error)
    {
      console.error('Error sending LIQUID_ANGLE_' + index + ':', error);
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
  async function OnInputCycleTimespan()
  {
    var output = document.getElementById('valueCycleTimespan');
    var slider = document.getElementById("sliderCycleTimespan");
    output.innerHTML = slider.value + "ms";
  }

  // Will be called if new slider value is changed
  async function OnChangeCycleTimespan()
  {    
    try
    {
      var slider = document.getElementById("sliderCycleTimespan");
    
      // Send CYCLE_TIMESPAN
      var response = await fetch('http://' + document.location.host + '/control?CYCLE_TIMESPAN=' + slider.value,
      {
        method: 'PUT'
      });
      
      // Check for response
      if (!response.ok)
      {
        console.log("Send: CYCLE_TIMESPAN no connection..");
        if (confirm("The control is not connected. Reload page?"))
        {
          window.location.reload();
        }
        return;
      }
      
      console.log("Send: CYCLE_TIMESPAN successful");
      
      // Set alive timestamp
      lastAliveTimestamp = Date.now();
    }
    catch (error)
    {
      console.error('Error sending CYCLE_TIMESPAN:', error);
    }
  }

})();

