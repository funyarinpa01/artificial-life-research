class SerializedOrg {
    constructor(org) {
        this.born = org.born
        this.id = org.id
        this.children = Array.from(org.children).map(child => new SerializedOrg(child))
    }
}


function castDown(map) {
    return new SerializedOrg(map.get(0))
}


function display(map) {
    let root = castDown(map)
    console.log(root)
    var nodes = tree.nodes(root),
    links = tree.links(nodes);

    var link = svg.selectAll(".link")
        .data(links)
    .enter().append("path")
        .attr("class", "link")
        .attr("d", diagonal);


    var decade = svg.selectAll(".decade")
        .data([20000, 40000, 60000, 80000, 100000, 120000, 140000, 160000, 180000, 200000])
    .enter().append("g")
        .attr("class", "decade")

    decade.append("circle")
        .attr("r", function(d){return yearToY(d)});

    decade.append("text")
        .attr("dy", ".31em")
        .attr("transform", function(d) { return "translate(-10,-"+yearToY(d)+")" })
        .text(function(d) { return d; });

    svg.selectAll(".title")
    .data(["Ancestor"])
    .enter().append("text")
        .attr("class", "title")
        .attr("transform", function(d) { return "translate(-22,20)" })
        .text(function(d) { return d; });

    var node = svg.selectAll(".node")
        .data(nodes)
    .enter().append("g")
        .attr("class", "node")
        .attr("transform", function(d) { return "rotate(" + (d.x - 90) + ")translate(" + getY(d) + ")"; })

    var c = d3.scale.category10();
    node.append("circle")
        .attr("r", 4.5)
        .attr("stroke", function(d){return c(d.depth)});
    node.append("text")
        .attr("dy", ".31em")
        .attr("text-anchor", function(d) { return d.x < 180 ? "start" : "end"; })
        .attr("transform", function(d) { return d.x < 180 ? "translate(8)" : "rotate(180)translate(-8)"; })
        .text(function(d) { return d.id; });

    d3.select(self.frameElement).style("height", diameter - 150 + "px");

}


//let root = organismsMap.values()

var diameter = 800;

// The year when the person was born to the radius
function yearToY(year){ return (year/20000)*50;}
function getY(d) { 
  //return d.y;
  return yearToY(d.born);
};

var tree = d3.layout.tree()
    .size([360, diameter / 2 - 120])
    .separation(function(a, b) { return (a.parent == b.parent ? 1 : 2) / a.depth; });

var diagonal = d3.svg.diagonal.radial()
    .source(function(d){ 
        if(d.source.id=="") 
          return {x: d.target.x, y: getY(d.source)}; 
        else 
          return {x: d.source.x, y: getY(d.source)};
      })
    .target(function(d){ return {x: d.target.x, y: getY(d.target)};})
    .projection(function(d) { return [d.y, d.x / 180 * Math.PI]; });

var svg = d3.select("#treeZone").append("svg")
    .attr("width", diameter)
    .attr("height", diameter)
  .append("g")
    .attr("transform", "translate(" + diameter / 2 + "," + diameter / 2 + ")");



///////////////////////////





