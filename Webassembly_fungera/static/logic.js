
const container = document.querySelector(".containerSim")
const SVGchildrenSpace = document.getElementById("childrenSpace")
const iterationsInModal = document.getElementById("modalIteration") 
const organismsMap = new Map()
const commands = ['.', ':', 'a', 'b', 'c', 'd', 'x', 'y', '^', 'v', '>', '<', '&', '?', '1', '0', '-', '+', '~', 'L', 'W', '@', '$', 'S', 'P'];

let popOver;
let runIterations;




fillMemory(100)
Module.onRuntimeInitialized = () => {
    runIterations = Module.cwrap("run", "number", ["number"])
}

class OrganismDOM {
    constructor(id, startX, startY, width, height) {
        

        this.id = id
        this.startX = startX
        this.startY = startY
        this.width = width
        this.height = height
        this.isSelected = false
        this.children = []
        this.init()


        // Later

        // organism.stackTop = stackTop
        // organism.ptr = [ptrx, ptry]
        // organism.delta = [deltaX, deltaY]
        // organism.a = [ax, ay]
        // organism.b = [bx, by]
        // organism.c = [cx, cy]
        // organism.d = [dx, dy]
        // organism.errors = errors
        // organism.reproduction_cycle = reproduction_cycle
        // organism.parent = ...
    }

    isTouched(x, y) {
        return this.startX <= x && this.startX + this.height >= x && this.startY <= y && this.startY + this.width >= y 
    }

    toggleDOMCol(color) {
        let currRow = container.childNodes[this.startX]
        let currCol = currRow.childNodes[this.startY]
        for (let i = this.startX; i < this.startX + this.height; i++) {
            for (let j = this.startY; j < this.startY + this.width; j++) {
                currCol.classList.toggle("marked" + color)
                currCol = currCol.nextSibling
            }
            currRow = currRow.nextSibling
            currCol = currRow.childNodes[this.startY]
        }
        console.log(`Marking area of organism start from (${this.startX}, ${this.startY}) 
        with width ${this.width} and height ${this.height}`)
    }

    select() {
        this.toggleDOMCol("Red")
        this.hookPopOver()
        this.selectChildrenDOM()
        this.isSelected = true
    }

    unselect() {
        this.toggleDOMCol("Red")
        this.clearPopOver()
        this.unselectChildrenDOM()
        this.isSelected = false
    }

    init() {
        this.toggleDOMCol("Blue")
    }

    displayInfo() {
        return `parent id: ${this.parentId}
        size: [${this.width},${this.height}]
        coors: [${this.startX},${this.startY}]
        amount of stack elements: ${this.stackTop}
        children amount: ${this.children.length}
        next command: [${this.ptr}]
        delta: [${this.delta}]
        errors: ${this.errors}
        reproduction cycle: ${this.reproduction_cycle}`
    }

    getStartElement() {
        return container.childNodes[this.startX].childNodes[this.startY]
    }

    getCenterCoordinates() {
        return [this.startX + Math.floor(this.height / 2), this.startY + Math.floor(this.width / 2)]   
    }

    getCenterElement() {
        return container.childNodes[this.startX + Math.floor(this.height / 2)].childNodes[this.startY + Math.floor(this.width / 2)]
    }

    getPtrElement() {
        return container.childNodes[this.startX + this.ptrx].childNodes[this.startY + this.ptry]
    }

    hookPopOver() {
        popOver = new bootstrap.Popover(this.getStartElement(), {
            content: this.displayInfo(), trigger: "manual", title: `Organism ${this.id}`, placement: "left"
        })
        popOver.show()
    }

    clearPopOver() {
        if (popOver !== undefined){
            popOver.hide()
        } 
    }

    selectChildrenDOM() {
        for (let org of this.children) {
            connectOrganisms(this.getCenterCoordinates(), org.getCenterCoordinates())
        }
    }

    unselectChildrenDOM() {
        SVGchildrenSpace.textContent = ""
    }

    addChildren(org) {
        this.children.push(org)
    }
}

function connectOrganisms(coordinates1, coordinates2, color="#149c61") {
    let line = document.createElementNS("http://www.w3.org/2000/svg", 'line');
    line.setAttribute("x1", (coordinates1[1]*20).toString())
    line.setAttribute("x2", (coordinates2[1]*20).toString())
    line.setAttribute("y1", (coordinates1[0]*20).toString())
    line.setAttribute("y2", (coordinates2[0]*20).toString())
    line.setAttribute("stroke", color)
    line.setAttribute("stroke-width", "2px")
    line.classList.add("line")
    SVGchildrenSpace.appendChild(line)
    console.log(`Connecting coordinates ${coordinates1} and ${coordinates2}`)
}


function fillMemory(numOfRow) {
    let row = document.createElement("div")
    let cell = document.createElement("div")
    cell.classList.add("cell")
    row.classList.add("row")

    for (let j = 0; j < numOfRow; j++) {
        cell.textContent = "."
        row.appendChild(cell)
        cell = cell.cloneNode()
    }

    for (let i = 0; i < numOfRow; i++) {
        row = row.cloneNode(true)
        container.appendChild(row)
    }

    // make svg space the same size
    SVGchildrenSpace.setAttribute("width", container.clientWidth) 
    SVGchildrenSpace.setAttribute("height", container.clientHeight) 

}


container.addEventListener("click", e => {
    let x, y;
    x = Math.floor(e.pageY / 20)
    y = Math.floor(e.pageX / 20)
    console.log(`Fixed click on [${x}, ${y}]`)
    let chosen;
    for (organism of organismsMap.values()) {
        if (organism.isSelected) {
            organism.unselect()
        } else if (organism.isTouched(x, y)){
            chosen = organism
        }
    }
    if (chosen !== undefined) chosen.select();
    // add warning when none touched
})

// org1 = new OrganismDOM("a", 5, 5, 6, 6)
// org2 = new OrganismDOM("b", 20, 20, 6, 6)
// org1.addChildren(org2)
// organismsMap.set("a", org1)
// organismsMap.set("b", org2)


function getCell(x, y) {
    return container.childNodes[x].childNodes[y]
}

function updateCell(x, y, sym) {
    let cell = getCell(x, y);
    if (cell !== undefined) {
        cell.textContent = sym
    }
}


function updateDOMOrganisms(id, startX, startY, width, height, ptrx, ptry, deltaX, deltaY,
     stackTop, errors, reproduction_cycle, parentId) {
    // hardcoded shit
    if (id !== 0) {
        temp = width
        width = height
        height = temp
    }
    //
    let organism;
    let parent = organismsMap.get(parentId)
    if (organismsMap.has(id)) {
        organism = organismsMap.get(id)
    } else {
        organism = new OrganismDOM(id, startX, startY, width, height)
        organismsMap.set(id, organism)
        if (parent !== undefined) {
            parent.addChildren(organism)
            organism.parentId = parentId
        }
    }

    organism.stackTop = stackTop
    organism.ptr = [ptrx, ptry]
    organism.delta = [deltaX, deltaY]
    // organism.a = [ax, ay]
    // organism.b = [bx, by]
    // organism.c = [cx, cy]
    // organism.d = [dx, dy]
    organism.errors = errors
    organism.reproduction_cycle = reproduction_cycle
}


