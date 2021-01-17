
const container = document.querySelector(".containerSim")
const SVGchildrenSpace = document.getElementById("childrenSpace")
const iterationsInModal = document.getElementById("modalIteration") 
const deadDOM = document.getElementById("dead")
const aliveDOM = document.getElementById("alive")
const sizesDOM = document.getElementById("sizes")
const maxChildrenDOM = document.getElementById("max-children")
const organismsMap = new Map()
const commands = ['.', ':', 'a', 'b', 'c', 'd', 'x', 'y', '^', 'v', '>', '<', '&', '?', '1', '0', '-', '+', '~', 'L', 'W', '@', '$', 'S', 'P'];

const size = 500
let newIds = new Set()
let popOver;
let runIterations;
let deadAmount = 0;
let aliveAmount = 0;
let orgWithMostChildren;
let genotypes = [[23, 17]];



fillMemory(size)
Module.onRuntimeInitialized = () => {
    runIterations = Module.cwrap("run", "number", ["number"])
}

class OrganismDOM {
    constructor(id, startX, startY, width, height, born) {
        

        this.id = id
        this.startX = startX
        this.startY = startY
        this.width = width
        this.height = height
        this.isSelected = false
        this.children = new Set()
        this.born = born;
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
        //console.log(`Marking area of organism start from (${this.startX}, ${this.startY}) 
        //with width ${this.width} and height ${this.height}`)
    }

    select() {
        this.toggleDOMCol("Red")
        this.hookPopOver()
        this.selectChildrenDOM()
        this.selectParentDOM()
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

    kill() {
        this.toggleDOMCol("Blue")
        this.getPtrElement().classList.remove("markedGreen")
    }

    displayInfo() {
        return `parent id: ${this.parentId}
        born: ${this.born}
        size: [${this.width},${this.height}]
        coors: [${this.startX},${this.startY}]
        amount of stack elements: ${this.stackTop}
        children amount: ${this.children.size}
        next command: [${this.ptr}]
        delta: [${this.delta}]
        errors: ${this.errors}
        reproduction cycle: ${this.reproduction_cycle}`
    }

    getCenterCoordinates() {
        return [this.startX + Math.floor(this.height / 2), this.startY + Math.floor(this.width / 2)]   
    }

    getOrganismCell(x, y) {
        return getCell(this.startX + x, this.startY + y)
    }

    getStartElement() {
        return this.getOrganismCell(0, 0)
    }

    getCenterElement() {
        return this.getOrganismCell(Math.floor(this.height / 2), Math.floor(this.width / 2))    
    }


    getPtrElement() {
        return this.getOrganismCell(this.ptr[0], this.ptr[1])
    }

    markNextCommand(ptrx, ptry) {
        if (this.ptr !== undefined) {
            this.getPtrElement().classList.remove("markedGreen")
        }
        this.getOrganismCell(ptrx, ptry).classList.add("markedGreen")  
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
            connectCoors(this.getCenterCoordinates(), org.getCenterCoordinates())
        }
    }

    selectParentDOM() {
        if (organismsMap.has(this.parentId)) {
            let parent = organismsMap.get(this.parentId)
            connectCoors(this.getCenterCoordinates(), parent.getCenterCoordinates(), "#eaed24")
        }
    }

    unselectChildrenDOM() {
        SVGchildrenSpace.textContent = ""
    }
}

function connectCoors(coordinates1, coordinates2, color="#149c61") {
    let line = document.createElementNS("http://www.w3.org/2000/svg", 'line');
    line.setAttribute("x1", (coordinates1[1]*20).toString())
    line.setAttribute("x2", (coordinates2[1]*20).toString())
    line.setAttribute("y1", (coordinates1[0]*20).toString())
    line.setAttribute("y2", (coordinates2[0]*20).toString())
    line.setAttribute("stroke", color)
    line.setAttribute("stroke-width", "4px")
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


// Cells
function checkOutOfBound(x, y) {
    return (x <= size && x > 0 & y <= size && y >= 0)
}

function getCell(x, y) {
    if (checkOutOfBound(x, y)) {
        return container.childNodes[x].childNodes[y]
    }
}

function updateCell(x, y, sym) {
    let cell = getCell(x, y);
    if (cell !== undefined) {
        cell.textContent = sym
    }
}
// 

// run after each cycle of iterations
function clearDead(queueIds, organisms) {
    let organism, parent;
    for (key of organisms.keys()) {
        organism = organisms.get(key)
        //console.log(JSON.stringify(organism))
        parent = organisms.get(organism.parentId)
        if (!queueIds.has(key)) {
            // parent may be already dead, so we check
            if (parent !== undefined && parent.children.has(organism)) {
                parent.delete(organism)
            }
            organisms.delete(key)
            organism.kill()
            deadAmount++;
        } else {
            if (key != 0) parent.children.add(organism)
        }
    }
    aliveAmount = organisms.size
    orgWithMostChildren = getOrgWithMostChilren(organisms)
    updateInfoDOM()
    newIds.clear()
    console.log(organisms)
}

function getOrgWithMostChilren(organisms) {
    let children_amount = -1;
    let most_chld_org;
    for (org of organisms.values()) {
        if (org.children.size > children_amount) {
            children_amount = org.children.size
            most_chld_org = org
        }
    }
    return most_chld_org
}

function updateGenotypes(width, height) {
    for (el of genotypes) {
        if (el[0] != width || el[1] != height) {
            genotypes.push([width, height])
            break;
        }
    }
}

function updateInfoDOM() {
    deadDOM.innerHTML = deadAmount
    aliveDOM.innerHTML= aliveAmount
    sizesDOM.innerHTML = JSON.stringify(genotypes)
    maxChildrenDOM.innerHTML = orgWithMostChildren.children.size
}


function updateDOMOrganisms(id, startX, startY, width, height, ptrx, ptry, deltaX, deltaY,
     stackTop, errors, reproduction_cycle, parentId, born) {
    // hardcoded shit
    if (id !== 0) {
        temp = width
        width = height
        height = temp
    }
    let organism;
    if (organismsMap.has(id)) {
        organism = organismsMap.get(id)
    } else {
        organism = new OrganismDOM(id, startX, startY, width, height, born)
        organism.parentId = parentId
        organismsMap.set(id, organism)
        updateGenotypes(width, height)
    }

    if (organism.getOrganismCell(ptrx, ptry) !== undefined) {
        organism.markNextCommand(ptrx, ptry)
        organism.ptr = [ptrx, ptry]
    }

    organism.stackTop = stackTop
    organism.delta = [deltaX, deltaY]
    organism.errors = errors
    organism.reproduction_cycle = reproduction_cycle
    // organism.a = [ax, ay]
    // organism.b = [bx, by]
    // organism.c = [cx, cy]
    // organism.d = [dx, dy]
}



container.addEventListener("click", e => {
    let x, y;
    x = Math.floor(e.pageY / 20)
    y = Math.floor(e.pageX / 20)
    let chosen;
    for (organism of organismsMap.values()) {
        if (organism.isSelected) {
            organism.unselect()
        } else if (organism.isTouched(x, y)){
            chosen = organism
        }
    }
    if (chosen !== undefined) chosen.select();
})
