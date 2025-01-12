/**
 * Includes a draggable doughnut chart
 *
 * @author    Florian Staeblein
 * @date      2024/01/28
 * @copyright © 2024 Florian Staeblein
 */

(function ()
{
  var extend = function (out)
  {
    out = out || {};

    for (var index = 1; index < arguments.length; index++)
    {
      if (!arguments[index])
      {
        continue;
      }

      for (var key in arguments[index])
      {
        if (arguments[index].hasOwnProperty(key))
        {
          out[key] = arguments[index][key];
        }
      }
    }

    return out;
  };

  // Generate function for doughnut chart
  var DraggableDoughnutchart = function ()
  {
  }

  // Setup function for doughnut chart
  DraggableDoughnutchart.prototype.Setup = function (setupValues)
  {
    // DraggableDoughnutchart ablegen
    var doughnutchart = this;
  
    // Extend setup values
    setupValues = extend({}, this.defaults, setupValues);

    // Set canvas and gerenate context
    this.canvas = setupValues.canvas;
    this.context = setupValues.canvas.getContext("2d");

    // Check if context has been generated
    if (!this.context)
    {
      console.log('Error: DraggableDoughnutchart needs an html5 canvas.');
      return;
    }

    // Set all remaining variables
    this.data = setupValues.data;
    this.draggedSegment = null;
    this.hoveredIndex = -1;
    this.innerRadius = setupValues.innerRadius;
    this.outerRadius = setupValues.outerRadius;
    this.minAngle = setupValues.minAngle;
    this.online = false;
    this.onchange = setupValues.onchange;
    this.onshift = setupValues.onshift;

    // Bind appropriate events
    this.canvas.addEventListener('touchstart', function (e)
    {
      TouchStart(e);
      e.preventDefault();
    });
    this.canvas.addEventListener('touchmove', function (e)
    {
      TouchMove(e);
      e.preventDefault();
    });
    document.addEventListener('touchend', function (e)
    {
      TouchEnd(e);
    });

    this.canvas.addEventListener('mousedown', TouchStart);
    this.canvas.addEventListener('mousemove', TouchMove);
    document.addEventListener('mouseup', TouchEnd);
    
    // Returns mouse location in relation to the client area
    function GetMouseLocation(event)
    {
      var rect = doughnutchart.canvas.getBoundingClientRect();

      if (event.clientX)
      {
        return {
          x: event.clientX - rect.left,
          y: event.clientY - rect.top
        }
      }
      else
      {
        return {
          x: event.targetTouches[0].clientX - rect.left,
          y: event.targetTouches[0].clientY - rect.top
        }
      }
    };

    // Will be called if mouse will be clicked or touch starts
    function TouchStart(event)
    {
      var dragLocation = GetMouseLocation(event);
      
      // Set dragged segment
      doughnutchart.draggedSegment = doughnutchart.GetTarget(dragLocation);
      
      if (doughnutchart.draggedSegment)
      {
        // Set index of dragged segment as hovered
        doughnutchart.hoveredIndex = doughnutchart.draggedSegment.index;
      }
    };

    // Will be called if mouse will be released or touch ends
    function TouchEnd()
    {
      // Reset dragged segment
      if (doughnutchart.draggedSegment)
      {
        var changedIndex = doughnutchart.draggedSegment.index;
        doughnutchart.draggedSegment = null;
        doughnutchart.Draw();
      }
    };

    // Will be called if mouse or touch is moving
    function TouchMove(event)
    {
      var dragLocation = GetMouseLocation(event);
      
      // If there is a dragged segment shift angle
      if (doughnutchart.draggedSegment)
      {
        var draggedSegment = doughnutchart.draggedSegment;

        var dx = dragLocation.x - draggedSegment.centerX;
        var dy = dragLocation.y - draggedSegment.centerY;
        
        // Calculate angle to x-axis
        var grabbedAngle = radiansToDegrees(Math.atan2(dy, dx));
      
        // Context draws 0° to the right x-Axis
        // We want to start the chart at the top, so we have to add 90°
        var grabbedAngle90 = normaliseAngle(grabbedAngle + 90.0);
        
        // Make integer: double to int
        var grabbedAngle90_int = parseInt(grabbedAngle90, 10);
        
        doughnutchart.ShiftSelectedAngle(grabbedAngle90_int);
        doughnutchart.Draw();
      }
      // If there is no dragged segment only draw hover index
      else
      {
        var hoveredTarget = doughnutchart.GetTarget(dragLocation);
        if (hoveredTarget)
        {
          // Set index of hovered doughnut part and draw without user input flag
          doughnutchart.hoveredIndex = hoveredTarget.index;
          doughnutchart.Draw();
        }
        else if (doughnutchart.hoveredIndex !== -1)
        {
          // Reset index of hovered doughnut part and draw without user input flag
          doughnutchart.hoveredIndex = -1;
          doughnutchart.Draw();
        }
      }
    };

    // Draw current doughnut chart
    this.Draw();
  };
  
  
  // Set the names of the doughnut chart parts
  DraggableDoughnutchart.prototype.Setonline = function (online)
  {
    if (this.online != online)
    {
      this.online = online;
      this.Draw(); 
    }
  };
  
  // Set the names of the doughnut chart parts
  DraggableDoughnutchart.prototype.Setnames = function (names)
  {
    // Check for correct length
    if (this.data.length !== names.length)
    {
      return;
    }
    
    // Set new names
    for (var index = 0; index < this.data.length; index++)
    {
      this.data[index].label = names[index];
    }
    
    // Draw
    this.Draw(); 
  };
  
  // Set the colors of the doughnut chart parts
  DraggableDoughnutchart.prototype.Setcolors = function (colors)
  {
    // Check for correct length
    if (this.data.length !== colors.length)
    {
      return;
    }
    
    // Set new colors
    for (var index = 0; index < this.data.length; index++)
    {
      this.data[index].color = colors[index];
    }
    
    // Draw
    this.Draw();
  };
  
  // Set the angles of the doughnut chart parts in degrees
  DraggableDoughnutchart.prototype.Setangles = function (angles)
  {
    // Check for correct length
    if (this.data.length !== angles.length)
    {
      return;
    }
    
    // Set new angles
    for (var index = 0; index < this.data.length; index++)
    {
      this.data[index].angle = angles[index];
    }
    
    // Draw
    this.Draw();
  };

  // Move angle specified by index and angle in degrees
  DraggableDoughnutchart.prototype.MoveAngle = function (index, angle)
  {
    var geometry = this.GetGeometry();
    this.draggedSegment =
    {
      index: index,
      centerX: geometry.centerX,
      centerY: geometry.centerY,
      startingAngles: this.data.map(function (segment)
      {
        return segment.angle;
      }),
      angleDragDistance: 0
    };

    this.ShiftSelectedAngle(this.data[index].angle + angle);
    this.draggedSegment = null;
    this.Draw();
  };

  // Gets percentage for slice with current index
  DraggableDoughnutchart.prototype.GetSliceSizePercentage = function (index)
  {
    var arcSize = this.GetArcSize(index);    
    return 100.0 * arcSize / 360.0;
  };

  // Gets the geometry of the doughnut chart in the canvas
  DraggableDoughnutchart.prototype.GetGeometry = function ()
  {
    var centerX = Math.floor(this.canvas.width / 2);
    var centerY = Math.floor(this.canvas.height / 2);
    return {
      centerX: centerX,
      centerY: centerY,
      innerRadius: Math.min(centerX, centerY) * this.innerRadius,
      outerRadius: Math.min(centerX, centerY) * this.outerRadius
    }
  };

  // Returns a segment to drag if given a close enough location
  DraggableDoughnutchart.prototype.GetTarget = function (targetLocation)
  {
    var geometry = this.GetGeometry();
    var startingAngles = [];

    var closest =
    {
      index: -1,
      distance: 9999999,
      angle: null
    };

    for (var segmentIndex = 0; segmentIndex < this.data.length; segmentIndex++)
    {
      startingAngles.push(this.data[segmentIndex].angle);

      // Get location on doughnut chart
      var dx = targetLocation.x - geometry.centerX;
      var dy = targetLocation.y - geometry.centerY;
      
      // Calculate the distance to the center
      var distanceToCenter = Math.sqrt(dx ** 2 + dy ** 2);
      
      // Check radius inbounds
      if (distanceToCenter < geometry.innerRadius || distanceToCenter > geometry.outerRadius)
      {
        return;
      }
      
      // Calculate angle to x-axis
      var grabbedAngle = radiansToDegrees(Math.atan2(dy, dx));
      
      // Context draws 0° to the right x-Axis
      // We want to start the chart at the top, so we have to add 90°
      var grabbedAngle90 = normaliseAngle(grabbedAngle + 90.0);
      
      var distance = Math.abs(smallestSignedAngleBetween(grabbedAngle90, this.data[segmentIndex].angle));

      if (distance < closest.distance)
      {
        closest.index = segmentIndex;
        closest.distance = distance;
        closest.angle = grabbedAngle90;
      }
    }

    // If closest distance is lower than 10° -> target found, return closest angle
    if (closest.distance < 10)
    {
      return {
        index: closest.index,
        centerX: geometry.centerX,
        centerY: geometry.centerY,
        startingAngles: startingAngles,
        angleDragDistance: 0
      }
    }
    // No target found, return null
    else
    {
      return null;
    }
  };
  
  // Returns drawable segments
  DraggableDoughnutchart.prototype.GetArcSize = function (index)
  {
    // Check for length
    if (index >= this.data.length)
    {
      return 0;
    }
    
    // Only one segment found, draw for the full 360 degrees
    if (this.data.length == 1)
    {
      return 360;
    }
    
    // Get arcSize
    var nextAngleIndex = (index + 1) % this.data.length;
    var arcSize = this.data[nextAngleIndex].angle - this.data[index].angle;
    
    // Check for 360° overflow
    if (arcSize <= 0)
    {
      arcSize += 360;
    }

    return arcSize;
  };

  // Draws the doughnutchart
  DraggableDoughnutchart.prototype.Draw = function ()
  {
    var doughnutchart = this;
    var context = doughnutchart.context;
    var canvas = doughnutchart.canvas;
    context.clearRect(0, 0, canvas.width, canvas.height);

    var geometry = this.GetGeometry();

    // Draw each segment
    for (index = 0; index < doughnutchart.data.length; index++)
    {
      doughnutchart.DrawSegment(context, doughnutchart, geometry, index);
    }
    
    // Draw each label
    for (index = 0; index < doughnutchart.data.length; index++)
    {
      doughnutchart.DrawLabel(context, doughnutchart, geometry, index);
    }
    
    // Draw node if hovered segment
    if (doughnutchart.hoveredIndex !== -1)
    {
      doughnutchart.DrawNode(context, doughnutchart, geometry, doughnutchart.hoveredIndex);
    }
    
    // Draw online/offline state
    context.save();
    context.translate(geometry.centerX, geometry.centerY);
    var fontsize = Math.floor(context.canvas.height / 25);
    context.textAlign = "center";
    context.font = fontsize + "pt Helvetica";
    context.fillText(doughnutchart.online ? "Online" : "Offline", 0, fontsize / 2);
    context.restore();
    
    doughnutchart.onchange();
  };

  // *INTERNAL USE ONLY*
  // Moves the selected angle to a new angle
  DraggableDoughnutchart.prototype.ShiftSelectedAngle = function (newAngle)
  {
    var doughnutchart = this;
    if (!doughnutchart.draggedSegment)
    {
      return;
    }
    var draggedSegment = doughnutchart.draggedSegment;

    // Get starting angle of the target
    var startingAngle = draggedSegment.startingAngles[draggedSegment.index];

    // Get diff from grabbed target start (as -180 to +180)
    var angleDragDistance = smallestSignedAngleBetween(newAngle, startingAngle);
    
    // Set next angle drag distance
    draggedSegment.angleDragDistance = angleDragDistance;

    // Set the new angle:
    var newDraggedAngle = normaliseAngle(startingAngle + angleDragDistance);
        
    // Get "previous index" (counter clockwise) and "next index" (clockwise)
    // Distance to previous
    var previousIndex = mod(parseInt(draggedSegment.index) - 1, doughnutchart.data.length);
    var distanceDragToPrevious = smallestSignedAngleBetween(newAngle, draggedSegment.startingAngles[previousIndex]);
    var angleBorderPrevious = normaliseAngle(draggedSegment.startingAngles[previousIndex] + doughnutchart.minAngle);
    
    // Distance to next
    var nextIndex = mod(parseInt(draggedSegment.index) + 1, doughnutchart.data.length);
    var distanceDragToNext = smallestSignedAngleBetween(newAngle, draggedSegment.startingAngles[nextIndex]);
    var angleBorderNext = normaliseAngle(draggedSegment.startingAngles[nextIndex] - doughnutchart.minAngle);
    
    // Determine active border index and angle
    var realShiftedDistance = 0;
    if (!isBetween(angleBorderPrevious, newDraggedAngle, angleBorderNext))
    {
      // Determine which border to set the angle to
      var borderedAngle = Math.abs(distanceDragToNext) < Math.abs(distanceDragToPrevious) ? angleBorderNext : angleBorderPrevious;
      
      // Calculate real shifted distance
      realShiftedDistance = borderedAngle - doughnutchart.data[draggedSegment.index].angle
      
      // Set the new angle
      doughnutchart.data[draggedSegment.index].angle = borderedAngle;
    }
    else
    {
      // Calculate real shifted distance
      realShiftedDistance = newDraggedAngle - doughnutchart.data[draggedSegment.index].angle
      
      // Set the new angle
      doughnutchart.data[draggedSegment.index].angle = newDraggedAngle;
    }
    
    //console.log("Next=" + nextIndex + ", Angle=" + distanceDragToNext + " | Previous=" + previousIndex + ", Angle=" + distanceDragToPrevious);
    
    // Check for angle changes
    if (realShiftedDistance != 0)
    {
      doughnutchart.onshift(draggedSegment.index, realShiftedDistance);
    }
  };
  
  // Draws a single segment
  DraggableDoughnutchart.prototype.DrawSegment = function (context, doughnutchart, geometry, index)
  {
    var startingAngle = doughnutchart.data[index].angle;
    var endingAngle = doughnutchart.data[index].angle + doughnutchart.GetArcSize(index);
    
    // Context draws 0° to the right x-Axis
    // We want to start the chart at the top, so we have to subtract 90°
    var startingAngle90 = normaliseAngle(startingAngle - 90.0);
    var endingAngle90 = normaliseAngle(endingAngle - 90.0);
    
    // Draw coloured segment path (context.arc() uses angle in rad)
    context.save();
    context.beginPath();
    context.moveTo(geometry.centerX, geometry.centerY);
    context.arc(geometry.centerX, geometry.centerY, geometry.outerRadius, degreesToRadians(startingAngle90), degreesToRadians(endingAngle90), false);
    context.arc(geometry.centerX, geometry.centerY, geometry.innerRadius, degreesToRadians(endingAngle90), degreesToRadians(startingAngle90), true);
    context.closePath();

    // Fill coloured segment path
    context.fillStyle = doughnutchart.data[index].color;
    context.fill();
    context.restore();
  };

  // Draws a single label
  DraggableDoughnutchart.prototype.DrawLabel = function (context, doughnutchart, geometry, index)
  {
    var startingAngle = doughnutchart.data[index].angle;
    
    // Context draws 0° to the right x-Axis
    // We want to start the chart at the top, so we have to subtract 90°
    var startingAngle90 = normaliseAngle(startingAngle - 90.0);
    
    // Draw label on top (context.rotate() uses angle in rad)
    context.save();
    context.translate(geometry.centerX, geometry.centerY);
    context.rotate(degreesToRadians(startingAngle90));

    var fontSize = Math.floor(context.canvas.height / 35);
    var dx = geometry.outerRadius - (geometry.outerRadius - geometry.innerRadius) / 2;
    var dy = geometry.centerY / 12;

    context.textAlign = "center";
    context.font = fontSize + "pt Helvetica";
    context.fillText(doughnutchart.data[index].label, dx, dy);
    context.restore();
  };

  // Draws a single segment
  DraggableDoughnutchart.prototype.DrawNode = function (context, doughnutchart, geometry, index)
  {
    var startingAngle = doughnutchart.data[index].angle;
    
    // Context draws 0° to the right x-Axis
    // We want to start the chart at the top, so we have to subtract 90°
    var startingAngle90 = normaliseAngle(startingAngle - 90.0);
    
    var locationFrom = polarToCartesian(startingAngle90, geometry.innerRadius);
    var locationTo = polarToCartesian(startingAngle90, geometry.outerRadius);
    
    // Draws a drag node on the segment
    context.save();
    context.translate(geometry.centerX, geometry.centerY);
    context.fillStyle = '#DDDDDD';
    
    // Draw line between two segments
    context.beginPath();
    context.moveTo(locationFrom.x, locationFrom.y);
    context.lineTo(locationTo.x, locationTo.y);
    context.lineWidth = 3;
    context.stroke();

    // Draw inner node
    context.beginPath();
    context.lineWidth = 1;
    context.arc(locationFrom.x, locationFrom.y, 6, 0, 2 * Math.PI, true);
    context.fill();
    context.stroke();
    
    // Draw outer node
    context.beginPath();
    context.lineWidth = 1;
    context.arc(locationTo.x, locationTo.y, 6, 0, 2 * Math.PI, true);
    context.fill();
    context.stroke();
    
    context.restore();
  };

  DraggableDoughnutchart.prototype.defaults =
  {
    data: [
      { label: 'Segment 1', color: "#2665da", angle: 0   },
      { label: 'Segment 2', color: "#6dd020", angle: 120 },
      { label: 'Segment 3', color: "#f9df18", angle: 240 } ],
    innerRadius: 0.5,
    outerRadius: 0.9,
    minAngle: 6,    
    onchange: function () { },
    onshift: function (index, increments) { }
  };

  window.DraggableDoughnutchart = DraggableDoughnutchart;

  //-------------------------------------------------
  // Utilities + Constants
  //-------------------------------------------------
  // Calculates 0-360° to 0-2Pi   
  function degreesToRadians(degrees)
  {
    return (degrees * Math.PI) / 180.0;
  };
  
  // Calculates 0-2Pi to 0-360°
  function radiansToDegrees(rads)
  {
    return (rads * 180.0) / Math.PI;
  };

  // Calculates a signed angle difference from source angle to target angle
  function smallestSignedAngleBetween(target, source)
  {
    var target_rad = degreesToRadians(target);
    var source_rad = degreesToRadians(source);
    return radiansToDegrees(Math.atan2(Math.sin(target_rad - source_rad), Math.cos(target_rad - source_rad)));
  };

  // Modulo function
  function mod(n, m)
  {
    return ((n % m) + m) % m;
  };

  // Normalises an angle from 0-2Pi
  function normaliseAngle(angle)
  {
    return mod(angle, 360);
  };

  // Returns cartesian coordinates from angle and radius
  function polarToCartesian(angle, radius)
  {
    return {
      x: radius * Math.cos(degreesToRadians(angle)),
      y: radius * Math.sin(degreesToRadians(angle))
    }
  };
  
  // Returns true, if angle is between previous angle and next angle
  function isBetween(previousAngle, angle, nextAngle)
  {
    var distToZero = 360 - previousAngle;
    
    var tempPreviousAngle = normaliseAngle(previousAngle + distToZero);
    var tempAngle = normaliseAngle(angle + distToZero);
    var tempNextAngle = normaliseAngle(nextAngle + distToZero);
    
    return tempPreviousAngle < tempAngle && tempAngle < tempNextAngle;
    
  };
  
})();
