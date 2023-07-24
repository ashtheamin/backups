function getNewTaskId() {
    if (localStorage.getItem("taskID") === null) {
        localStorage.setItem("taskID", JSON.stringify(-1));
    }
    const oldID = JSON.parse(localStorage.getItem("taskID"));
    const newID = oldID + 1;
    localStorage.setItem("taskID", JSON.stringify(newID));
    return newID;
}

function addTask() {
    if (document.getElementById('taskInputForm')['addTask'].value < 1) {
        return;
    }
    let taskArray = JSON.parse(localStorage.getItem('taskArray'));
    const newTaskLabel = document.getElementById('taskInputForm')['addTask'].value
    const newTask = {"id": getNewTaskId(), "label": newTaskLabel, "timeAdded": Date.now()};  
    taskArray.push(newTask);
    localStorage.setItem('taskArray', JSON.stringify(taskArray));
}
document.getElementById('taskInputForm').addEventListener("submit", function(event) {
    addTask();
});

function removeTask(taskList, taskDiv) {
    let taskArray = JSON.parse(localStorage.getItem("taskArray"));

    let counter = 0;
    for (task of taskArray) {
        if (task.id.toString() === taskDiv.getAttribute("id").toString()) {
            taskArray.splice(counter, 1);
            localStorage.setItem("taskArray", JSON.stringify([]));
            localStorage.setItem("taskArray", JSON.stringify(taskArray));
            break;
        }
        counter = counter + 1;
    }
    taskList.removeChild(taskDiv);
    return;
}

function renderTaskList() {
    let taskArray = JSON.parse(localStorage.getItem('taskArray'));
    if (localStorage.getItem('taskArray') === null) {
        localStorage.setItem('taskArray', JSON.stringify(new Array));
        taskArray = JSON.parse(localStorage.getItem('taskArray'));
    }
    
    for (task of taskArray) {
        const taskList = document.getElementById("taskList");
        const taskDiv = document.createElement("div");
        taskDiv.setAttribute("id", task.id);
        taskDiv.setAttribute("class", "taskDiv")



        const taskDivTimeAdded = document.createElement("li");
        /*
        let time = new Date(Date.now() - task.timeAdded);
        taskDivTimeAdded.textContent = time;
        taskDivTimeAdded.setAttribute("class", "taskDivElement");
        taskDiv.appendChild(taskDivTimeAdded);
        */
        const taskDivLabel = document.createElement("li");
        taskDivLabel.textContent = task.label;
        taskDivLabel.setAttribute("class", "taskDivElement");
        taskDiv.appendChild(taskDivLabel);
   
        const taskDivRemoveButton = document.createElement("button");
        taskDivRemoveButton.textContent = "Remove"
        taskDivRemoveButton.setAttribute("class", "taskDivElement");
        
        taskDiv.appendChild(taskDivRemoveButton);
        taskList.appendChild(taskDiv);

        taskDivRemoveButton.addEventListener("click", function(taskDivRemoveButton) {
            removeTask(taskList, taskDiv);
            return;
        })
    }
}

function downloadTasks() {
    let file = new Blob([localStorage.getItem("taskArray")], {type: 'text/json'});
    let a = document.createElement("a");
    a.href = URL.createObjectURL(file);
    
    a.download = "tasks.json"
    a.click();
}

document.getElementById("downloadButton").addEventListener("click", function (event) {
    downloadTasks()
})

/*This code is from Mozilla Developer Network. Big thanks to Mozilla.
https://developer.mozilla.org/en-US/docs/Web/API/File_API*/
function uploadTasks() {
    const upload = document.getElementById("upload");
    const [file] = upload.files;

    if (file) {
        const reader = new FileReader();
        reader.addEventListener("load", () => {
            localStorage.setItem("taskArray", reader.result)
            renderTaskList();
        });
        reader.readAsText(file);
    }
}

document.getElementById("upload").addEventListener("change", function(event) {
    uploadTasks();
}) 

document.getElementById("uploadButton").addEventListener("click", function() {
    document.getElementById("upload").click();
})


renderTaskList();