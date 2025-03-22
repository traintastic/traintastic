/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2024-2025 Reinder Feenstra
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

function Throttle(parent, id)
{
  this.id = id;
  this.trainId = '';
  this.message = null;
  this.messageTimeout = null;

  var createButton = function (action, innerText, className)
  {
    var e = document.createElement('button');
    e.innerText = innerText;
    e.className = className;
    e.setAttribute('throttle-id', id);
    e.setAttribute('action', action);
    e.onclick = function ()
    {
      var msg = {
        'throttle_id': parseInt(this.getAttribute('throttle-id')),
        'action': this.getAttribute('action'),
      };
      if(msg.action == 'faster' || msg.action == 'slower' || msg.action == 'stop')
      {
        msg.immediate = localStorage.immediateSpeedControl != 'false';
      }
      tm.send(msg);
    };
    return e;
  };

  var createDiv = function (className, children)
  {
    var e = document.createElement('div');
    e.className = className;
    children.forEach(function (c, _)
    {
      e.appendChild(c);
    });
    return e;
  };

  var createTrainSelect = function (className)
  {
    var e = document.createElement('select');
    e.className = className;
    e.setAttribute('throttle-id', id);
    e.setAttribute('name', 'train_select');
    e.onchange = function ()
    {
      if(this.value != '')
      {
        tm.send({
          'throttle_id': parseInt(this.getAttribute('throttle-id')),
          'action': 'acquire',
          'train_id': this.value,
          'steal': false,
        });
      }
      else
      {
        tm.send({
          'throttle_id': parseInt(this.getAttribute('throttle-id')),
          'action': 'release',
          'stop': localStorage.throttleStopOnRelease != 'false',
        });
      }
    };
    return e;
  }

  var createSpan = function (id, text = '', className = '')
  {
    var e = document.createElement('span');
    e.id = id;
    e.className = className;
    e.innerText = text;
    return e;
  }

  var addRemoveClass = function (e, add, className)
  {
    if(add && !e.classList.contains(className))
    {
      e.classList.add(className);
    }
    else if(!add)
    {
      e.classList.remove(className);
    }
  }

  var formatSpeed = function (value, unit)
  {
    if(unit == 'kmph')
    {
      return value.toFixed(0) + ' km/h';
    }
    if(unit == 'mph')
    {
      return value.toFixed(0) + ' mph';
    }
    if(unit == 'mps')
    {
      return value.toFixed(1) + ' m/s';
    }
    return value + ' ' + unit;
  }

  var functions = createDiv('flex-column flex-resize', []);

  var throttle = createDiv('p1 stretch', [
    createTrainSelect('control stretch'),
    createDiv('flex-row', [
      createSpan('', 'Speed:', 'text-right pr1'),
      createSpan('throttle-' + id + '-actual-speed', '', 'stretch'),
      createSpan('', 'Target:', 'stretch text-right pr1'),
      createSpan('throttle-' + id + '-target-speed', '', 'stretch'),
      /* not yet supported:
            createSpan('', 'Limit:', 'stretch text-right pr1'),
            createSpan('throttle-' + id + '-target-limit', '∞', 'stretch'),
      */
    ]),
    createDiv('flex-row', [
      createDiv('flex-resize', [
        functions
      ]),
      createDiv('flex-resize', [
        createDiv('flex-column stretch', [
          createButton('faster', '+', 'control stretch'),
          createButton('slower', '-', 'control stretch'),
        ])
      ])
    ]),
    createDiv('flex-row', [
      createDiv('flex-resize', [createButton('reverse', '<', 'control stretch')]),
      createDiv('flex-resize', [createButton('stop', '0', 'control stretch')]),
      createDiv('flex-resize', [createButton('forward', '>', 'control stretch')]),
    ]),
    createButton('estop', 'EStop', 'control stretch red'),
  ]);
  parent.appendChild(throttle);

  this.setTrainList = function (list)
  {
    var train_select = throttle.querySelector('select[name=train_select]');

    train_select.innerHTML = '';
    train_select.appendChild(document.createElement('option'));
    list.forEach(function (train, _)
    {
      var option = document.createElement('option');
      option.value = train['id'];
      option.innerText = train['name'];
      train_select.appendChild(option);
    });
  };

  this.showMessage = function (message)
  {
    var layout = [];
    if(message.tag == 'can_not_activate_train')
    {
      throttle.querySelector('select[name=train_select]').value = this.trainId;
    }

    text = document.createElement('p');
    text.innerText = message['text'];
    layout.push(text);

    if(message['tag'] == 'already_acquired')
    {
      var e = document.createElement('button');
      e.innerText = 'Steal';
      e.setAttribute('throttle-id', id);
      e.setAttribute('train-id', throttle.querySelector('select[name=train_select]').value);
      e.onclick = function ()
      {
        var throttleId = parseInt(this.getAttribute('throttle-id'));
        tm.send({
          'throttle_id': throttleId,
          'action': 'acquire',
          'train_id': this.getAttribute('train-id'),
          'steal': true,
        });
        tm.throttles[throttleId].clearMessage();
      };
      layout.push(e)
    }

    this.clearMessage();
    this.message = createDiv('message ' + message['type'], layout);
    throttle.appendChild(this.message);
    this.messageTimeout = window.setTimeout(function (throttle) { throttle.clearMessage(); }, 5000, this);
  }

  this.clearMessage = function ()
  {
    if(this.message)
    {
      throttle.removeChild(this.message);
      window.clearTimeout(this.messageTimeout);
      this.message = null;
    }
  }

  this.setTrain = function (train)
  {
    functions.replaceChildren();
    var buttons = throttle.querySelectorAll('button.control');
    if(train)
    {
      this.updateDirectionButtonState(train.direction, train.is_stopped);
      this.setSpeed(train.speed.value, train.speed.unit);
      this.setThrottleSpeed(train.throttle_speed.value, train.throttle_speed.unit);
      buttons.forEach(function (button) { button.disabled = false; });
      this.trainId = train.id;
      train.functions.forEach(function (group)
      {
        var groupbox = document.createElement('fieldset');
        groupbox.className = 'flex-resize';
        var legend = document.createElement('legend');
        legend.innerText = group.name;
        groupbox.appendChild(legend);
        group.items.forEach(function (func)
        {
          var btn = document.createElement('button');
          btn.innerText = func.name;
          btn.className = 'control square';
          if(func.value)
          {
            btn.classList.add('active');
          }
          btn.setAttribute('throttle-id', id);
          btn.setAttribute('vehicle-id', group.id);
          btn.setAttribute('function-number', func.number);
          btn.onclick = function ()
          {
            tm.send({
              'throttle_id': parseInt(this.getAttribute('throttle-id')),
              'vehicle_id': this.getAttribute('vehicle-id'),
              'function_number': parseInt(this.getAttribute('function-number')),
              'action': 'toggle_function',
            });
          };
          groupbox.appendChild(btn);
        });
        functions.appendChild(groupbox);
      });
    }
    else
    {
      buttons.forEach(function (button) { button.disabled = true; });
      this.trainId = '';
    }
    throttle.querySelector('select[name=train_select]').value = this.trainId;
  }

  this.setDirection = function (direction)
  {
    this.updateDirectionButtonState(direction, this.stopped)
  }

  this.setIsStopped = function (stopped)
  {
    this.updateDirectionButtonState(this.direction, stopped)
  }

  this.updateDirectionButtonState = function (direction, stopped)
  {
    this.direction = direction;
    this.stopped = stopped;

    const fwd = (direction == 'forward');
    const rev = (direction == 'reverse');

    var btn = throttle.querySelector('button[action=forward]');
    addRemoveClass(btn, fwd, 'active');
    btn.disabled = !stopped && !fwd;

    btn = throttle.querySelector('button[action=reverse]');
    addRemoveClass(btn, rev, 'active');
    btn.disabled = !stopped && !rev;
  }

  this.setSpeed = function (value, unit)
  {
    document.getElementById('throttle-' + this.id + '-actual-speed').innerText = formatSpeed(value, unit);
  }

  this.setThrottleSpeed = function (value, unit)
  {
    document.getElementById('throttle-' + this.id + '-target-speed').innerText = formatSpeed(value, unit);
  }

  this.setFunctionValue = function (vehicleId, number, value)
  {
    var btn = throttle.querySelector('button[vehicle-id="' + vehicleId + '"][function-number="' + number + '"]');
    addRemoveClass(btn, value, 'active');
  }

  this.setTrain(null);
};

var tm = new function ()
{
  this.ws = null;
  this.throttles = []

  this.init = function ()
  {
    if(localStorage.throttleName)
    {
      document.getElementById('throttle_name').value = localStorage.throttleName;
    }
    if(localStorage.throttleStopOnRelease)
    {
      document.getElementById('stop_train_on_release').value = localStorage.throttleStopOnRelease;
    }
    if(localStorage.immediateSpeedControl)
    {
      document.getElementById('immediate_speed_control').value = localStorage.immediateSpeedControl;
    }

    document.getElementById('open_settings').onclick = function ()
    {
      var settings = document.getElementById('settings');
      if(settings.classList.contains('hide'))
      {
        settings.classList.remove('hide');
      }
      else
      {
        document.getElementById('close_settings').onclick();
      }
    };

    document.getElementById('close_settings').onclick = function ()
    {
      localStorage.throttleName = document.getElementById('throttle_name').value;
      localStorage.throttleStopOnRelease = document.getElementById('stop_train_on_release').checked;
      localStorage.immediateSpeedControl = document.getElementById('immediate_speed_control').checked;
      document.getElementById('settings').classList.add('hide');
      tm.connect();
    };

    if(localStorage.throttleName && localStorage.throttleStopOnRelease)
    {
      document.getElementById('settings').classList.add('hide');
      this.connect();
    }
  }

  this.add = function ()
  {
    this.connect();
    var e = document.getElementById('throttles');
    var id = 1;
    while(id in this.throttles) { id++; }
    this.throttles[id] = new Throttle(e, id);
    return this.throttles[id];
  }

  this.connect = function ()
  {
    if(!this.ws)
    {
      this.ws = new WebSocket((window.location.protocol == 'https' ? 'wss' : 'ws') + '://' + window.location.host + window.location.pathname);
      this.ws.onopen = function (ev)
      {
        console.log('onopen');
        document.getElementById('not-connected').classList.add('hide');
      };
      this.ws.onmessage = function (ev)
      {
        var msg = JSON.parse(ev.data);
        console.log('RX', msg);
        if(msg['event'] == 'world')
        {
          if(msg.name === null)
          {
            document.getElementById('throttles').replaceChildren();
            document.getElementById('throttles').classList.add('hide');
            document.getElementById('no-world').classList.remove('hide');
          }
          else
          {
            document.getElementById('throttles').classList.remove('hide');
            document.getElementById('no-world').classList.add('hide');
            if(!document.getElementById('throttles').hasChildNodes())
            {
              tm.add();
            }
            else
            {
              tm.throttles.forEach(function (throttle, _)
              {
                throttle.setTrainList([]); // clear train list
              });
            }

            tm.send({ 'action': 'get_train_list' });

            tm.throttles.forEach(function (throttle, _)
            {
              tm.send({
                'throttle_id': throttle.id,
                'action': 'set_name',
                'value': localStorage.throttleName + ' #' + throttle.id,
              });
              const trainId = localStorage['throttle' + throttle.id + 'TrainId'];
              if(trainId)
              {
                tm.send({
                  'throttle_id': throttle.id,
                  'action': 'acquire',
                  'train_id': trainId,
                  'steal': false,
                });
              }
            });
          }
        }
        else if(msg['event'] == 'train_list')
        {
          tm.throttles.forEach(function (throttle, _)
          {
            throttle.setTrainList(msg['list']);
          });
        }
        else if(msg['event'] == 'message')
        {
          var throttleId = msg['throttle_id'];
          if(throttleId)
          {
            tm.throttles[throttleId].showMessage(msg);
          }
        }
        else if(msg['event'] == 'train')
        {
          var throttleId = msg['throttle_id'];
          var train = msg['train'];
          var item = 'throttle' + throttleId + 'TrainId';
          if(train)
          {
            localStorage.setItem(item, train.id);
          }
          else
          {
            localStorage.removeItem(item);
          }
          tm.throttles[throttleId].setTrain(train);
        }
        else if(msg['event'] == 'direction')
        {
          tm.throttles[msg['throttle_id']].setDirection(msg['value']);
        }
        else if(msg['event'] == 'is_stopped')
        {
          tm.throttles[msg['throttle_id']].setIsStopped(msg['value']);
        }
        else if(msg['event'] == 'speed')
        {
          tm.throttles[msg['throttle_id']].setSpeed(msg['value'], msg['unit']);
        }
        else if(msg['event'] == 'throttle_speed')
        {
          tm.throttles[msg['throttle_id']].setThrottleSpeed(msg['value'], msg['unit']);
        }
        else if(msg['event'] == 'function_value')
        {
          tm.throttles[msg.throttle_id].setFunctionValue(msg.vehicle_id, msg.number, msg.value);
        }
      };
      this.ws.onclose = function ()
      {
        console.log('onclose');
        document.getElementById('not-connected').classList.remove('hide');
        document.getElementById('throttles').classList.add('hide');
        document.getElementById('no-world').classList.add('hide');
        tm.ws = null;
        setTimeout(function () { tm.connect(); }, 1000);
      }
      this.ws.onerror = function (err)
      {
        console.error('WebSocket error: ', err.message);
        tm.ws.close();
      };
    }
  }

  this.send = function (msg)
  {
    console.log('TX', msg);
    return this.ws.send(JSON.stringify(msg));
  };

  this.eStopAll = function ()
  {
    this.send({ 'action': 'estop_all' });
  };
}();

document.addEventListener("visibilitychange", function ()
{
  if(document.hidden)
  {
    tm.eStopAll();
  }
});
