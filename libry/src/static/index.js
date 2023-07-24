function downloadBooks() {
    var request = new XMLHttpRequest();
    request.open("GET", "/bookdb", false);
    request.send(null);
    return request.responseText;
}

function sendData() {
    const data = { data: "example" };

    fetch("/recievedata", {
    method: "POST", // or "PUT"
    headers: {
        "Content-Type": "application/json",
    },
    body: JSON.stringify(data),
    })
    .then(response => response.json())
    .then(data => {
    console.log("Success:", data);
    })
    .catch((error) => {
    console.error("Error:", error);
    });
}

let download = JSON.parse(downloadBooks());
const book_list = document.getElementById("books");
for (const book of download) {
    const title = document.createElement("h2");
    title.textContent = book["title"]
    book_list.appendChild(title);

    const year = document.createElement("h4");
    year.textContent = "Year of Publication: " + book["year"]
;
    book_list.appendChild(year)
    const description = document.createElement("p");
    description.textContent = book["description"]
    book_list.appendChild(description);
}