(function(){
  var script_url = new URL(document.currentScript.getAttribute("src"), window.location.href).href
  var parts = script_url.split('/', -1)
  const multiversion = parts.length >= 6;

  parts.pop()
  parts.pop()
  parts.pop()

  const component_idx = parts.length;

  function populate(info) {
    var menu = document.querySelector("#version-menu");
    var html = "<div>";

    if (info.versions.length >= 2) {
      var dropdown = document.querySelector("#versions-menu-dropdown-marker");
      if (dropdown != undefined) {
        dropdown.innerHTML = " ▼";
      }
    }

    for (const element of info.versions) {
      var href = "#";

      if (multiversion) {
        href = window.location.href.split('/', -1);
        href[component_idx] = element;
        href = href.join("/");
      }

      var item = '<a href="' + href + '">' + element + '</a>';

      if ("${DOCS_VERSION}" === element) {
        item = "<b>" + item + "</b>";
      }

      html += item;
    }
    menu.innerHTML = html;
  }

  if (!multiversion) {
    populate({
      versions: ["4.0", "3.x"]
    })
    return;
  }

  var versions_json = (parts.concat(["versions.json"])).join("/")
  function reqListener() {
    var data = JSON.parse(this.responseText);
    populate(data)
  }

  function reqError(_err) { }
  var oReq = new XMLHttpRequest();
  oReq.onload = reqListener;
  oReq.onerror = reqError;
  oReq.open('GET', versions_json, true);
  oReq.send();
})()
