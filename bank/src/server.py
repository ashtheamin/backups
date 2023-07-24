from flask import Flask, request, make_response, jsonify
from database import *

app = Flask(__name__, static_url_path=('/static'))

@app.route('/')
def index():
    return app.send_static_file('index.html')

@app.route('/recieveUserLoginForm', methods=['POST'])
def recieveUserLoginForm():
    response = make_response(app.send_static_file('redirectToIndex.html'))
    databaseInit()
    token = databaseUserLogin(request.form['email'], request.form['password']) #type: ignore
    if (token == None):
        response.set_cookie('userNotFound', "true")
        return response
    response.set_cookie('jwt', token)
    return response

@app.route('/recieveToken', methods=['POST', 'GET'])
def recieveToken():
    token = request.cookies.get('jwt') #type: ignore
    response = make_response()
    databaseInit()
    exists = databaseTokenValidateExistence(token)
    if exists == True:
        response.set_cookie('jwtValid', 'true')
    else:
        response.set_cookie('jwtValid', 'false')
    return response

@app.route('/recieveUserRegistrationForm', methods=['POST'])
def recieveUserRegistrationForm():
    response = make_response(app.send_static_file('redirectToIndex.html'))
    databaseInit()
    databaseUserNew(request
    .form['name'], request.form['email'] ,request.form['password1']) #type: ignore
    token = databaseUserLogin(request.form['email'], request.form['password1']) #type: ignore
    response.set_cookie('jwt', token)
    return response

@app.route("/fetchUserInformationByToken", methods=['POST', 'GET'])
def fetchUserInformationByToken():
    return jsonify(databaseUserGetByToken(request.cookies.get('jwt'))) #type: ignore

@app.route("/fetchUserAccountsByToken", methods=['POST', 'GET'])
def fetchUserAccountsByToken():
    return jsonify(databaseAccountsGetByToken(request.cookies.get('jwt'))) #type:ignore

@app.route("/accountNew", methods=["POST", "GET"])
def accountNew():
    response = make_response(app.send_static_file('redirectToIndex.html'))
    token = request.cookies.get('jwt') #type: ignore
    databaseAccountNew(token, "0")
    return response

@app.route("/accountRemove", methods=['GET', 'POST'])
def accountRemove():
    response = make_response(app.send_static_file('redirectToIndex.html'))
    token = request.cookies.get('jwt') #type: ignore
    databaseAccountDeleteByID(token, request.get_json()['accountID'])
    return response

@app.route("/requestLoan", methods=['POST'])
def requestLoan():
    response = make_response(app.send_static_file('redirectToIndex.html'))
    token = request.cookies.get('jwt') #type: ignore
    accountID = request.form['accountID']
    loanAmount = request.form['loanAmount']
    response.set_cookie("loanApproved", 
    databaseLoanRequest(token, accountID, loanAmount))
    return response

@app.route("/transferFunds", methods=["POST"])
def transferFunds():
    response = make_response(app.send_static_file('redirectToIndex.html'))
    token = request.cookies.get('jwt') #type: ignore
    fromAccountID = request.form['fromAccountID']
    toAccountID = request.form['toAccountID']
    transferAmount = request.form['transferAmount']
    print(fromAccountID, toAccountID, transferAmount)
    #response.set_cookie("fundsTransferSuccessful", 
    databaseAccountTransferFunds(token, fromAccountID, toAccountID, transferAmount)
    return response