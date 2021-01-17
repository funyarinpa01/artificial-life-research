
const simInfoModal = new bootstrap.Modal(document.getElementById('simInfo'))
const treeContainer = new bootstrap.Modal(document.getElementById("tree"))
console.log("init modal")
const slider = document.getElementById("sliderIteration")
const sliderOutput = document.getElementById("sliderOutput")
// Default
sliderOutput.innerHTML = Math.floor((parseInt(slider.max) + parseInt(slider.min)) / 2)
const showInfobtn = document.getElementById("showInfobtn")
const organismsInfoContainer = document.getElementById("organismsInfoContainer")
const runBtn = document.getElementById("runbtn")
let keeplooping = true


function loop() {
    if (!keeplooping) {
        alert("Stop iterations")
        return
    }
    runIterations(1)
    window.setTimeout(loop, 0)
}

function alert_finish(iteration) {
    alert(`Finished on ${iteration}`)
}

slider.oninput = function() {
    sliderOutput.innerHTML = this.value
}

function updateIteration(iteration) {
    iterationsInModal.textContent = iteration
}

showInfobtn.addEventListener("click", () => {
    organismsInfoContainer.classList.toggle("fadein")
})

runBtn.addEventListener("click", () => {
    simInfoModal.hide()
    //runIterations(parseInt(slider.value))
    window.setTimeout(() => {runIterations(parseInt(slider.value))}, 300)
})

document.addEventListener("keydown", (e) => {
    keeplooping = false
    if (e.keyCode == 32) {
        simInfoModal.show()
    } else if (e.keyCode == 66) {
        treeContainer.show()
        display(organismsMap)
    } else if (e.keyCode == 68) {
        runIterations(1);
    } else if (e.keyCode == 83) {
        runIterations(5000);
    } else if (e.keyCode == 65) {
        runIterations(5000);
    } else if (e.keyCode == 87) {
        runIterations(15000)
    } else if (e.keyCode == 67) {
        keeplooping = true
        loop()
    }
})