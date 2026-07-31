const ItaskInput = document.getElementById("Itask");
const IaddBtn = document.getElementById("Iadd");
const ItaskList = document.getElementById("ItaskList");

// Load Import tasks
let Itasks = JSON.parse(localStorage.getItem("Itasks")) || [];

// Display saved Import tasks
Itasks.forEach(Itask => IcreateTask(Itask));

// Add Import task
IaddBtn.addEventListener("click", () => {
    const ItaskText = ItaskInput.value.trim();

    if (!ItaskText) return;

    Itasks.push(ItaskText);
    localStorage.setItem("Itasks", JSON.stringify(Itasks));

    IcreateTask(ItaskText);

    ItaskInput.value = "";
});

// Create Import task
function IcreateTask(text) {

    const ItaskItem = document.createElement("li");

    ItaskItem.innerHTML = `
        <span>${text}</span>
        <button class="delete">Delete</button>
    `;

    ItaskList.appendChild(ItaskItem);

    const IdeleteBtn = ItaskItem.querySelector(".delete");

    IdeleteBtn.addEventListener("click", (e) => {

        e.stopPropagation();

        ItaskItem.remove();

        const Iindex = Itasks.indexOf(text);

		if (Iindex !== -1) {
			Itasks.splice(Iindex, 1);
		}

        localStorage.setItem("Itasks", JSON.stringify(Itasks));
    });

    ItaskItem.addEventListener("click", () => {
        ItaskItem.classList.toggle("completed");
    });
}



// ================= EXPORT LIST =================

const EtaskInput = document.getElementById("Etask");
const EaddBtn = document.getElementById("Eadd");
const EtaskList = document.getElementById("EtaskList");

// Load Export tasks
let Etasks = JSON.parse(localStorage.getItem("Etasks")) || [];

// Display saved Export tasks
Etasks.forEach(Etask => EcreateTask(Etask));

// Add Export task
EaddBtn.addEventListener("click", () => {
    const EtaskText = EtaskInput.value.trim();

    if (!EtaskText) return;

    Etasks.push(EtaskText);
    localStorage.setItem("Etasks", JSON.stringify(Etasks));

    EcreateTask(EtaskText);

    EtaskInput.value = "";
});

// Create Export task
function EcreateTask(text) {

    const EtaskItem = document.createElement("li");

    EtaskItem.innerHTML = `
        <span>${text}</span>
        <button class="delete">Delete</button>
    `;

    EtaskList.appendChild(EtaskItem);

    const EdeleteBtn = EtaskItem.querySelector(".delete");

    EdeleteBtn.addEventListener("click", (e) => {

        e.stopPropagation();

        EtaskItem.remove();

        const Eindex = Etasks.indexOf(text);

		if (Eindex !== -1) {
			Etasks.splice(Eindex, 1);
		}

		localStorage.setItem("Etasks", JSON.stringify(Etasks));
    });

    EtaskItem.addEventListener("click", () => {
        EtaskItem.classList.toggle("completed");
    });
}