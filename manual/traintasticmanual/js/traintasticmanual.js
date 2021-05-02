// TOC scroll spy:
(function()
{
  'use strict';

  var nodes = document.querySelectorAll("h1,h2");
  var headings = {};

  Array.prototype.forEach.call(nodes, function(e)
    {
      if(e.id == '')
        return;

      var marginTop = parseInt(window.getComputedStyle(e).marginTop.replace('px',''));
      headings[e.id] = e.offsetTop - marginTop;
    });

  window.onscroll = function()
    {
      var scrollPosition = document.documentElement.scrollTop || document.body.scrollTop;

      var id = '';
      for(var k in headings)
        if(scrollPosition < headings[k])
          break;
        else
          id = k;

      var e;
      if(e = document.querySelector('a.selected'))
        e.setAttribute('class', '');
      if(e = document.querySelector('a[href*=' + id + ']'))
        e.setAttribute('class', 'selected');

    };
})();
