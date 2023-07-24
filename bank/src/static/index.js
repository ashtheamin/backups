function renderLoginForm() {
    const loginForm = document.createElement('form');
    loginForm.setAttribute('method', 'post');
    loginForm.setAttribute('action', '/recieveUserLoginForm');
    loginForm.setAttribute('style', 'display: inline-block;')
    loginForm.addEventListener("submit", function() {
        a = document.createElement("a")
        a.href = "/"
        a.click()
    })

    const header = document.createElement('h4');
    header.textContent = 'Login to Bank';

    const emailLabel = document.createElement('label');
    emailLabel.textContent = 'Email'
    const email = document.createElement('input');
    email.setAttribute('type', 'email');
    email.setAttribute('name', 'email');
    email.setAttribute('id', 'email');
    email.setAttribute('required', 'true');
    email.setAttribute('autocomplete', 'current-email')

    const passwordLabel = document.createElement('label');
    passwordLabel.textContent = 'Password';
    const password = document.createElement('input');
    password.setAttribute('type', 'password');
    password.setAttribute('name', 'password');
    password.setAttribute('id', 'password');
    password.setAttribute('required', 'true')
    password.setAttribute('autocomplete', 'current-password')

    const submit = document.createElement('input');
    submit.setAttribute('type', 'submit');
    submit.setAttribute('class', 'submit')

    const register = document.createElement('button');
    register.setAttribute('style', 'display: inline-block;')
    register.textContent = 'Register a new user'
    register.addEventListener('click', function() {
        localStorage.setItem('registrationRequested', 'true')
        location.reload()
    })
    
    loginForm.appendChild(header);
    loginForm.appendChild(emailLabel);
    loginForm.appendChild(email);
    loginForm.appendChild(passwordLabel);
    loginForm.appendChild(password);
    loginForm.appendChild(submit);

    document.getElementsByTagName('body')[0].appendChild(loginForm);
    document.getElementsByTagName('body')[0].appendChild(document.createElement('div'));

    if (getCookie('userNotFound') == 'true') {
        p = document.createElement("p");
        p.textContent = "User not found, or password is incorrect. Try again."
        document.getElementsByTagName('body')[0].appendChild(p);
    }
    document.getElementsByTagName('body')[0].appendChild(register);
}

function renderRegistrationForm() {
    const registrationForm = document.createElement('form');
    registrationForm.setAttribute('method', 'post');
    registrationForm.setAttribute('action', '/recieveUserRegistrationForm');
    registrationForm.setAttribute('style', 'display: inline-block;')

    const header = document.createElement('h4');
    header.textContent = 'Register to Bank';

    const nameLabel = document.createElement('label');
    nameLabel.textContent = 'Name'
    const name = document.createElement('input');
    name.setAttribute('type', 'text');
    name.setAttribute('name', 'name');
    name.setAttribute('id', 'name');
    name.setAttribute('required', 'true');
    name.setAttribute('autocomplete', 'current-name')

    const emailLabel = document.createElement('label');
    emailLabel.textContent = 'Email'
    const email = document.createElement('input');
    email.setAttribute('type', 'email');
    email.setAttribute('name', 'email');
    email.setAttribute('id', 'email');
    email.setAttribute('required', 'true');
    email.setAttribute('autocomplete', 'current-email')

    const passwordLabel1 = document.createElement('label');
    passwordLabel1.textContent = 'Password';
    const password1 = document.createElement('input');
    password1.setAttribute('type', 'password');
    password1.setAttribute('name', 'password1');
    password1.setAttribute('id', 'password1');
    password1.setAttribute('required', 'true')
    password1.setAttribute('autocomplete', 'current-password1')

    const submit = document.createElement('input');
    submit.setAttribute('type', 'submit');
    submit.setAttribute('class', 'submit')

    const returnToLogin = document.createElement('button');
    returnToLogin.textContent = 'Return To Login'
    returnToLogin.addEventListener('click', function() {
        localStorage.setItem('registrationRequested', 'false');
        location.reload()
    })
    
    registrationForm.appendChild(header);
    registrationForm.appendChild(nameLabel);
    registrationForm.appendChild(name);
    registrationForm.appendChild(emailLabel);
    registrationForm.appendChild(email);
    registrationForm.appendChild(passwordLabel1);
    registrationForm.appendChild(password1);
    registrationForm.appendChild(submit);

    document.getElementsByTagName('body')[0].appendChild(registrationForm);
    document.getElementsByTagName('body')[0].appendChild(document.createElement('div'));
    document.getElementsByTagName('body')[0].appendChild(returnToLogin);

    registrationForm.addEventListener('submit', function() {
        location.reload();
    })
}

function renderAddAccountsButton() {
    const accountsAddButton = document.createElement("button");
    accountsAddButton.textContent = "Create New Account";
    const accountsAddButtonConfirmation = document.createElement("div");
    const accountsAddButtonConfirmationYes = document.createElement("button");
    accountsAddButtonConfirmationYes.textContent = "Yes"
    const accountsAddButtonConfirmationNo = document.createElement("button");
    accountsAddButtonConfirmationNo.textContent = "No"
    accountsAddButtonConfirmation.appendChild(accountsAddButtonConfirmationYes);
    accountsAddButtonConfirmation.appendChild(accountsAddButtonConfirmationNo);

    accountsAddButtonConfirmationYes.addEventListener("click", function() {
        request = new XMLHttpRequest();
        request.open("POST", "/accountNew", false);
        request.send(null);
        accountsAddButtonConfirmation.remove();
        renderMainScreen();
    })

    accountsAddButtonConfirmationNo.addEventListener("click", function() {
        accountsAddButtonConfirmation.remove();
    })
    
   
    accountsAddButton.addEventListener("click", function(e) {   
        e = window.event || e;
        if (this === e.target) {   
            accountsAddButton.appendChild(accountsAddButtonConfirmation); 
        }
    });

    document.getElementsByTagName('body')[0].appendChild(accountsAddButton);
}

function renderAccountsListAccessLoan(accountDiv, accountID) {
    const loanForm = document.createElement("form");
    loanForm.setAttribute("style", "display: inline-block; padding: 0;");
    loanForm.setAttribute("method", "post");
    loanForm.setAttribute("action", "/requestLoan");

    const loanAmountLabel = document.createElement("label");
    loanAmountLabel.textContent = "Loan Amount";

    const loanAmount = document.createElement("input");
    loanAmount.setAttribute("type", "text");
    loanAmount.setAttribute("name", "loanAmount");
    loanAmount.setAttribute("id", "loanAmount");

    const loanAccount = document.createElement("input");
    loanAccount.setAttribute("style", "display: none");
    loanAccount.setAttribute("type", "text");
    loanAccount.setAttribute("name", "accountID");
    loanAccount.setAttribute("id", "accountID");
    loanAccount.value = accountID

    const loanSubmit = document.createElement("input");
    loanSubmit.setAttribute("style", "display: none");
    loanSubmit.setAttribute("type", "submit");

    loanForm.appendChild(loanAmountLabel);
    loanForm.appendChild(loanAmount);
    loanForm.appendChild(loanAccount);
    loanForm.appendChild(loanSubmit);

    if (getCookie('loanApproved') == "true") {
        const loanSuccess = document.createElement("label");
        loanSuccess.textContent = "Loan Granted";
        loanForm.appendChild(loanSuccess);
    }

    if (getCookie('loanApproved') == "false") {
        const loanDenied = document.createElement("label");
        loanDenied.textContent = "Loan Denied";
        loanForm.appendChild(loanDenied);
    }
    console.log(getCookie('loanApproved'));
    const loanHeader = document.createElement("h4");
    loanHeader.textContent = "Get loan from bank"
    accountDiv.appendChild(loanHeader);
    accountDiv.appendChild(loanForm);
    accountDiv.appendChild(document.createElement("div"))
}

function renderAccountsListTransferFunds(accountDiv, accountID) {
    const fundsTransferForm = document.createElement("form");
    fundsTransferForm.setAttribute("style", "display: inline-block; padding: 0%;");
    fundsTransferForm.setAttribute("method", "post");
    fundsTransferForm.setAttribute("action", "/transferFunds"); 

    const fundsTransferLabel = document.createElement("h4");
    fundsTransferLabel.textContent = "Transfer funds to account."

    const fundsTransferAmountLabel = document.createElement("label");
    fundsTransferAmountLabel.textContent = "Amount of $ to transfer.";
    const fundsTransferAmountInput = document.createElement("input");
    fundsTransferAmountInput.setAttribute("type", "text");
    fundsTransferAmountInput.setAttribute("id", "transferAmount");
    fundsTransferAmountInput.setAttribute("name", "transferAmount");

    const fundsTransferAccountLabel = document.createElement("label");
    fundsTransferAccountLabel.textContent = "To account ID."
    const fundsTransferInputToAccount = document.createElement("input");

    fundsTransferInputToAccount.setAttribute("type", "text");
    fundsTransferInputToAccount.setAttribute("id", "toAccountID");
    fundsTransferInputToAccount.setAttribute("name", "toAccountID");

    const fundsTransferFromAccount = document.createElement("input");
    fundsTransferFromAccount.setAttribute("style", "display: none");
    fundsTransferFromAccount.setAttribute("id", "fromAccountID");
    fundsTransferFromAccount.setAttribute("name", "fromAccountID");
    fundsTransferFromAccount.value = accountID;

    const fundsTransferFormSubmit = document.createElement("input");
    fundsTransferFormSubmit.setAttribute("type", "submit");
    fundsTransferFormSubmit.setAttribute("style", "display: none;");

    fundsTransferForm.appendChild(fundsTransferLabel);
    fundsTransferForm.appendChild(document.createElement('div'))
    fundsTransferForm.appendChild(fundsTransferAmountLabel);
    fundsTransferForm.appendChild(document.createElement('div'));
    fundsTransferForm.appendChild(fundsTransferAmountInput);
    fundsTransferForm.appendChild(fundsTransferAccountLabel);
    fundsTransferForm.appendChild(fundsTransferInputToAccount);
    fundsTransferForm.appendChild(fundsTransferFromAccount);
    fundsTransferForm.appendChild(fundsTransferFormSubmit);
    accountDiv.appendChild(fundsTransferForm);
    accountDiv.appendChild(document.createElement('div'));
}

function renderAccountsListDeleteButton(accountDiv, accountID) {
    const accountDeleteButton = document.createElement("button");
    accountDeleteButton.textContent = "Close Account";

    accountDeleteButton.addEventListener("click", function(e) {
        e = window.event || e;
        if (this === e.target) {  
            accountDeleteButton.appendChild(accountDeleteButtonConfirmationDiv); 
        }
    });

    const accountDeleteButtonConfirmationDiv = document.createElement("div");
    const accountDeleteButtonConfirmationDivYes = document.createElement("button");
    accountDeleteButtonConfirmationDivYes.textContent = "Yes"
    const accountDeleteButtonConfirmationDivNo = document.createElement("button");
    accountDeleteButtonConfirmationDivNo.textContent = "No"
    accountDeleteButtonConfirmationDiv.appendChild
    (accountDeleteButtonConfirmationDivYes);
    accountDeleteButtonConfirmationDiv.appendChild(
    accountDeleteButtonConfirmationDivNo);

    accountDeleteButtonConfirmationDivYes.addEventListener("click", function() {
        request = new XMLHttpRequest();
        request.open("POST", "/accountRemove");
        request.setRequestHeader("Content-Type", "application/json;charset=UTF-8");
        request.send(JSON.stringify({"accountID":accountID}));
        accountDeleteButton.removeChild(accountDeleteButtonConfirmationDiv);
        renderMainScreen();
    })

    accountDeleteButtonConfirmationDivNo.addEventListener("click", function() {
        accountDeleteButton.removeChild(accountDeleteButtonConfirmationDiv);
    })

    accountDiv.appendChild(accountDeleteButton);
}

function renderAccountsList(accountInformation) {
    const accountsList = document.createElement("ul");
    accountsList.setAttribute("id", "accountsList");
    if (accountInformation != null) {
        for (account of accountInformation) {
            const accountDiv = document.createElement("div");
            accountDiv.setAttribute("class", "accountDiv");
            const accountID = document.createElement("p");
            accountID.textContent = `Account ID: ${account['accountID']}`
            const accountBalance = document.createElement("h3");
            accountBalance.textContent = `Balance: $${account['balance']}`;
            accountDiv.appendChild(accountBalance);
            accountDiv.appendChild(accountID);
            renderAccountsListAccessLoan(accountDiv, account['accountID']);
            renderAccountsListTransferFunds(accountDiv, account['accountID']);
            renderAccountsListDeleteButton(accountDiv, account['accountID']);
            accountsList.appendChild(accountDiv);
        }
    }
    document.getElementsByTagName('body')[0].appendChild(accountsList);
}

function renderMainScreen() {
    document.getElementsByTagName('body')[0].innerHTML = ""

    var request = new XMLHttpRequest();
    request.open("GET", "/fetchUserInformationByToken", false);
    request.send(null);
    const userInformation = JSON.parse(request.responseText);

    request = new XMLHttpRequest();
    request.open("GET", "/fetchUserAccountsByToken", false);
    request.send(null);
    const accountInformation = JSON.parse(request.responseText);

    const header = document.createElement("h2");
    header.textContent = `Welcome, ${userInformation['name']}`;
    const accountsHeader = document.createElement("h3");
    accountsHeader.textContent = "Accounts";

    document.getElementsByTagName('body')[0].appendChild(header);
    document.getElementsByTagName('body')[0].appendChild(accountsHeader);
    renderAddAccountsButton();
    renderAccountsList(accountInformation);
}

// Get cookie function from StackOverFlow by allenhwkim
// https://stackoverflow.com/a/49224652
function getCookie(name) {
    let cookie = {};
    document.cookie.split(';').forEach(function(el) {
      let [k,v] = el.split('=');
      cookie[k.trim()] = v;
    })
    return cookie[name];
}

async function validateToken() {
    await fetch('/recieveToken').then().then()
}

// Wait for the login token to be validated, then render UI.
// Indentation looks like this to emphasise 
// that this is a "main function" to avoid confusion.
validateToken().then( function() {
if (getCookie('jwtValid') != 'true') {
    if (localStorage.getItem('registrationRequested') != 'true') {
        renderLoginForm();
    }
    else {
        renderRegistrationForm();
    }
}

else {
    renderMainScreen();
}})