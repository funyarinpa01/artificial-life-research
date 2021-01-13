const container = document.querySelector(".container")
const circle = document.querySelector(".circle")
const line = document.getElementById("line")

function appendNode(to, positionX, positionY, type) {
    let newNode = type.cloneNode(true)
    newNode.style.left = positionX + "px"
    newNode.style.top = positionY + "px"
    to.appendChild(newNode)
    return newNode

}

function drawLine(element1, element2) {
    posEl1 = element1.getBoundingClientRect()
    posEl2 = element2.getBoundingClientRect()
    newLine = line.cloneNode(true)
    newLine.setAttribute("x1", posEl1.left)
    newLine.setAttribute("x2", posEl2.left)
    newLine.setAttribute("y1", posEl1.top)
    newLine.setAttribute("y2", posEl2.top)
    line.parentNode.appendChild(newLine)
}

node = appendNode(container, 500, 500, circle)
drawLine(node, circle)