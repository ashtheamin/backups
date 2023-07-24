import jwt

bankTokenSecret = "theMostSecretCodeEver"

def tokenNew(userID):
    return jwt.encode(payload={
        "userID": userID
    },
    key=bankTokenSecret)
    
def tokenDecrypt(token):
    headerData = jwt.get_unverified_header(token)
    return jwt.decode(token, bankTokenSecret, algorithms=[headerData['alg']])

