
const simInfoModal = new bootstrap.Modal(document.getElementById('simInfo'))
console.log("init modal")
const slider = document.getElementById("sliderIteration")
const sliderOutput = document.getElementById("sliderOutput")
// Default
sliderOutput.innerHTML = Math.floor((parseInt(slider.max) + parseInt(slider.min)) / 2)
const showInfobtn = document.getElementById("showInfobtn")
const organismsInfoContainer = document.getElementById("organismsInfoContainer")
const runBtn = document.getElementById("runbtn")


slider.oninput = function() {
    sliderOutput.innerHTML = this.value
}

function updateIteration(iteration) {
    alert(`Finished on ${iteration} iteration`);
    iterationsInModal.textContent = iteration
}

showInfobtn.addEventListener("click", () => {
    organismsInfoContainer.classList.toggle("fadein")
})

runBtn.addEventListener("click", () => {
    simInfoModal.hide()
    setTimeout(() => {runIterations(parseInt(slider.value))}, 300)
})

document.addEventListener("keyup", (e) => {
    if (e.keyCode == 32) {
        simInfoModal.show()
    }
})