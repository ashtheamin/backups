# 2023 Copyright Ash Amin
# Functions are from and modified from the guide here:
# https://www.postgresqltutorial.com/postgresql-python/connect/
# Big thank you for the clear and coherent tutorial.

import psycopg2
from configparser import ConfigParser
import bcrypt

from bankToken import tokenNew, tokenDecrypt

# Function is from: https://www.postgresqltutorial.com/postgresql-python/connect/
def databaseConfig(filename='database.ini', section='postgresql'):
    # create a parser
    parser = ConfigParser()
    # read config file
    parser.read(filename)

    # get section, default to postgresql
    db = {}
    if parser.has_section(section):
        params = parser.items(section)
        for param in params:
            db[param[0]] = param[1]
    else:
        raise Exception('Section {0} not found in the {1} file'.format(section, filename))

    return db

# Function to test connection to database.
def databaseConnect():
    """ Connect to the PostgreSQL database server """
    conn = None
    try:
        # read connection parameters
        params = databaseConfig()

        # connect to the PostgreSQL server
        print('Connecting to the PostgreSQL database...')
        conn = psycopg2.connect(**params)
		
        # create a cursor
        cur = conn.cursor()
        
	# execute a statement
        print('PostgreSQL database version:')
        cur.execute('SELECT version()')

        # display the PostgreSQL database server version
        db_version = cur.fetchone()
        print(db_version)
       
	# close the communication with the PostgreSQL
        cur.close()
    except (Exception, psycopg2.DatabaseError) as error:
        print(error)
    finally:
        if conn is not None:
            conn.close()
            print('Database connection closed.')

# Initialise Database.
def databaseInit():
    """ create tables in the PostgreSQL database"""
    commands = [
        ("""
        CREATE TABLE users (
            userID SERIAL PRIMARY KEY,
            userName VARCHAR(255) NOT NULL,
            email VARCHAR UNIQUE NOT NULL,
            password VARCHAR NOT NULL
        )
        """),
        ("""
        CREATE TABLE accounts (
            accountID SERIAL PRIMARY KEY,
            userID SERIAL,
            balance VARCHAR,
            FOREIGN KEY (userID)
            REFERENCES users (userID)
            ON UPDATE CASCADE ON DELETE CASCADE
        )
        """),
        ("""
        CREATE TABLE transactions (
            transactionID SERIAL PRIMARY KEY,
            fromAccountID SERIAL,
            toAccountID SERIAL,
            amount VARCHAR,
            FOREIGN KEY (fromAccountID)
            REFERENCES accounts (accountID)
            ON UPDATE CASCADE ON DELETE CASCADE,
            FOREIGN KEY (toAccountID)
            REFERENCES accounts (accountID)
            ON UPDATE CASCADE ON DELETE CASCADE
        )
        """),
        ("""
        CREATE TABLE bank (
            balance VARCHAR
        )
        """),
        ("""
        CREATE TABLE loans (
            loanID SERIAL PRIMARY KEY,
            amount varchar,
            accountID SERIAL,
            FOREIGN KEY (accountID)
            REFERENCES accounts (accountID)
            ON UPDATE CASCADE ON DELETE CASCADE
        )
        """),]
    conn = None
    try:
        # read the connection parameters
        params = databaseConfig()
        # connect to the PostgreSQL server
        conn = psycopg2.connect(**params)
        cur = conn.cursor()
        # create table one by one
        for command in commands:
            cur.execute(command)
        # Store bank balance.
        cur.execute("""SELECT * FROM bank""")
        if cur.fetchone() == None:
            cur.execute("""INSERT INTO bank (balance) VALUES(%s)""", ("1000",))
        # close communication with the PostgreSQL database server
        cur.close()
        # commit the changes
        conn.commit()
    except (Exception, psycopg2.DatabaseError) as error:
        print(error)
    finally:
        if conn is not None:
            conn.close()

# Validate whether a token/user exists:
def databaseTokenValidateExistence(token):
    conn = None
    try:
        # read database configuration
        params = databaseConfig()

        # connect to the PostgreSQL database
        conn = psycopg2.connect(**params)

        # create a new cursor
        cur = conn.cursor()

        # Get the user ID.
        userID = tokenDecrypt(token)['userID']

        # Find the user.
        cur.execute("""SELECT * FROM users WHERE userid=(%s)""", (userID,))

        # Return true if the user in the token is found
        if (cur.fetchone() != None):
            conn.commit()
            cur.close()
            return True

        # commit the changes to the database
        conn.commit()
        # close communication with the database
        cur.close()

        # Return user not found.
        return False
    except (Exception, psycopg2.DatabaseError) as error:
        print(error)
    finally:
        if conn is not None:
            conn.close()

# Create a new user.
def databaseUserNew(name, email, password:str):
    conn = None
    try:
        # read database configuration
        params = databaseConfig()

        # connect to the PostgreSQL database
        conn = psycopg2.connect(**params)

        # create a new cursor
        cur = conn.cursor()

        # Generate salt and hashed password.
        hashedBytes:bytes = bcrypt.hashpw(password.encode('utf-8'), bcrypt.gensalt())
        hashed:str = hashedBytes.decode("utf-8")

        # Add the user
        cur.execute("""INSERT INTO users(userName, email, password)VALUES(%s,%s, %s) RETURNING userID;""", (name, email, hashed,))

        # commit the changes to the database
        conn.commit()
        # close communication with the database
        cur.close()
    except (Exception, psycopg2.DatabaseError) as error:
        print(error)
    finally:
        if conn is not None:
            conn.close()

# Login a user, return a JWT.
def databaseUserLogin(email, password):
    conn = None
    try:
        # read database configuration
        params = databaseConfig()
        # connect to the PostgreSQL database
        conn = psycopg2.connect(**params)

        # create a new cursor
        cur = conn.cursor()

        # If the email doesn't exist, return None:
        cur.execute("""SELECT * from users WHERE email=(%s);""", (email,))
        if cur.fetchone() == None:
            conn.commit()
            cur.close()
            return None

        # Get the hashed password for the email.
        # Also get the user ID
        cur.execute("""SELECT password, userID FROM users WHERE email=(%s);""", (email,))
        sql_return = cur.fetchone()
        storedPasswordHash = sql_return[0] # type:ignore
        userID = sql_return[1] # type:ignore

        # Generate salt and hashed password.
        if bcrypt.checkpw(password.encode('utf-8'), storedPasswordHash.encode('utf-8')):
            print("match")
            conn.commit()
            cur.close()
            return tokenNew(userID)
        else:
            conn.commit()
            cur.close()
            return None
    except (Exception, psycopg2.DatabaseError) as error:
        print(error)
    finally:
        if conn is not None:
            conn.close()

# Retrieve a user.
def databaseUserGetByToken(token):
    """
    Inputs: id:
    Outputs: User tuple (id, name)
    """
    sql = """SELECT userid, username, email FROM users WHERE userID=(%s);"""
    conn = None
    user = None
    try:
        # read database configuration
        params = databaseConfig()
        # connect to the PostgreSQL database
        conn = psycopg2.connect(**params)
        # create a new cursor
        cur = conn.cursor()
        # execute the SQL statement
        cur.execute(sql, (tokenDecrypt(token)['userID'],))
        user = cur.fetchone()

        # commit the changes to the database
        conn.commit()
        # close communication with the database
        cur.close()

        # Return the user information.
        return {
            "userID": user[0], #type: ignore
            "name": user[1], #type: ignore
            "email": user[2] #type: ignore
        }
    except (Exception, psycopg2.DatabaseError) as error:
        print(error)
    finally:
        if conn is not None:
            conn.close()

# Delete a user.
def databaseUserDeleteByID(userID):
    sql = """DELETE FROM users WHERE userID=(%s);"""
    conn = None
    user = None
    try:
        # read database configuration
        params = databaseConfig()
        # connect to the PostgreSQL database
        conn = psycopg2.connect(**params)
        # create a new cursor
        cur = conn.cursor()
        # execute the SQL statement
        cur.execute(sql, (userID,))

        # commit the changes to the database
        conn.commit()
        # close communication with the database
        cur.close()
    except (Exception, psycopg2.DatabaseError) as error:
        print(error)
    finally:
        if conn is not None:
            conn.close()

# Update a user's name.
def databaseUserUpdateNameByID(userID, name):
    sql = """UPDATE users SET userName=(%s) WHERE userID=(%s);"""
    conn = None
    user = None
    try:
        # read database configuration
        params = databaseConfig()
        # connect to the PostgreSQL database
        conn = psycopg2.connect(**params)
        # create a new cursor
        cur = conn.cursor()
        # execute the SQL statement
        cur.execute(sql, (name,userID,))

        # commit the changes to the database
        conn.commit()
        # close communication with the database
        cur.close()
    except (Exception, psycopg2.DatabaseError) as error:
        print(error)
    finally:
        if conn is not None:
            conn.close()

# Open an account for a user.
def databaseAccountNew(token, balance):
    sql = """INSERT INTO accounts(userID, balance)
             VALUES(%s, %s) RETURNING accountID;
             """
    conn = None
    account = None
    try:
        # read database configuration
        params = databaseConfig()
        # connect to the PostgreSQL database
        conn = psycopg2.connect(**params)
        # create a new cursor
        cur = conn.cursor()
        # execute the SQL statement
        cur.execute(sql, (tokenDecrypt(token)['userID'], balance))

        # commit the changes to the database
        conn.commit()
        # close communication with the database
        cur.close()
    except (Exception, psycopg2.DatabaseError) as error:
        print(error)
    finally:
        if conn is not None:
            conn.close()

    return account

# Update account balance.
def databaseAccountBalanceUpdateByID(accountID, balance):
    sql = """UPDATE accounts SET balance=(%s) WHERE accountID=(%s);"""
    conn = None

    try:
        # read database configuration
        params = databaseConfig()
        # connect to the PostgreSQL database
        conn = psycopg2.connect(**params)
        # create a new cursor
        cur = conn.cursor()
        # execute the SQL statement
        cur.execute(sql, (balance,accountID,))

        # commit the changes to the database
        conn.commit()
        # close communication with the database
        cur.close()
    except (Exception, psycopg2.DatabaseError) as error:
        print(error)
    finally:
        if conn is not None:
            conn.close()

# Delete account.
def databaseAccountDeleteByID(token, accountID):
    sql = """DELETE FROM accounts WHERE accountID=(%s) AND userID=(%s);"""
    conn = None
    user = None
    try:
        # read database configuration
        params = databaseConfig()
        # connect to the PostgreSQL database
        conn = psycopg2.connect(**params)
        # create a new cursor
        cur = conn.cursor()
        # execute the SQL statement
        cur.execute(sql, (accountID, tokenDecrypt(token)['userID']))

        # commit the changes to the database
        conn.commit()
        # close communication with the database
        cur.close()
    except (Exception, psycopg2.DatabaseError) as error:
        print(error)
    finally:
        if conn is not None:
            conn.close()

# Transfer funds between accounts.
def databaseAccountTransferFunds(token, fromAccountID, toAccountID, amountOfFundsToTransfer):
    conn = None
    try:
        # read database configuration
        params = databaseConfig()
        # connect to the PostgreSQL database
        conn = psycopg2.connect(**params)
        # create a new cursor
        cur = conn.cursor()

        # Check if the accounts exist.
        cur.execute("""SELECT balance FROM accounts WHERE accountID=(%s) AND userID=(%s);""", 
        (fromAccountID, tokenDecrypt(token)['userID']))
        if cur.fetchone() == None:
            conn.commit()
            cur.close()
            return "false"
        
        cur.execute("""SELECT balance FROM accounts WHERE accountID=(%s);""", (toAccountID,))
        if cur.fetchone() == None:
            conn.commit()
            cur.close()
            return "false"


        # Get the from account's balance
        fromAccountBalance = 0
        cur.execute("""SELECT balance FROM accounts WHERE accountID=(%s) AND userID=(%s);""", 
        (fromAccountID, tokenDecrypt(token)['userID']))
        fromAccountBalance = float(cur.fetchone()[0]) # type:ignore

        if fromAccountBalance < float(amountOfFundsToTransfer):
            conn.commit()
            cur.close()
            return "false"
        
        # Deduct the senders account's balance.
        fromAccountBalance = float(fromAccountBalance) - float(amountOfFundsToTransfer)
        cur.execute("""UPDATE accounts SET balance=(%s) where accountID=(%s)""", 
        ((fromAccountBalance), fromAccountID,))

        # Get the to account balance.
        toAccountBalance = 0
        cur.execute("""SELECT balance FROM accounts WHERE accountID=(%s);""", (toAccountID,))
        toAccountBalance = float(cur.fetchone()[0]) # type:ignore

        # Add the funds to the transferred account's balance.
        toAccountBalance = float(toAccountBalance) + float(amountOfFundsToTransfer)
        cur.execute("""UPDATE accounts SET balance=(%s) where accountID=(%s)""", (toAccountBalance, toAccountID,))

        # Store the transaction:
        cur.execute("""INSERT INTO transactions (fromAccountID, toAccountID, amount) VALUES (%s, %s, %s)""",
        (fromAccountID, toAccountID, amountOfFundsToTransfer,))
        # commit the changes to the database
        conn.commit()
        # close communication with the database
        cur.close()
        return "true"
    except (Exception, psycopg2.DatabaseError) as error:
        print(error)
    finally:
        if conn is not None:
            conn.close()

# Deposit funds into account.
def databaseAccountDepositFunds(accountID, amountToDeposit):
    conn = None
    try:
        # read database configuration
        params = databaseConfig()
        # connect to the PostgreSQL database
        conn = psycopg2.connect(**params)
        # create a new cursor
        cur = conn.cursor()

        # Check if the accounts exist.
        cur.execute("""SELECT balance FROM accounts WHERE accountID=(%s);""", (accountID,))
        sqlReturn = cur.fetchone()
        if sqlReturn == None:
            conn.commit()
            cur.close()
            return
        accountBalance = sqlReturn[0] # type:ignore
        
        # Deduct the account's balance.
        cur.execute("""UPDATE accounts SET balance=(%s) where accountID=(%s)""", ((accountBalance + amountToDeposit), accountID,))

        # commit the changes to the database
        conn.commit()
        # close communication with the database
        cur.close()
    except (Exception, psycopg2.DatabaseError) as error:
        print(error)
    finally:
        if conn is not None:
            conn.close()

# Recieve Account information
def databaseAccountsGetByToken(token):
    sql = """SELECT balance, accountID, userID FROM accounts WHERE userID=(%s);"""
    conn = None
    try:
        # read database configuration
        params = databaseConfig()
        # connect to the PostgreSQL database
        conn = psycopg2.connect(**params)
        # create a new cursor
        cur = conn.cursor()
        # execute the SQL statement
        cur.execute(sql, (tokenDecrypt(token)['userID'],))
        accounts = cur.fetchall()

        # Account list: 
        accountList = []
        for account in accounts:
            accountList.append({
                "balance": account[0],
                "accountID": account[1],
                "userID": account[2],
            })
            
        # commit the changes to the database
        conn.commit()
        # close communication with the database
        cur.close()

        # Return the account information.
        return accountList
    except (Exception, psycopg2.DatabaseError) as error:
        print(error)
    finally:
        if conn is not None:
            conn.close()


# Get loan from bank to add to user.
def databaseLoanRequest(token, accountID, loanAmount):
    conn = None
    try:
        print(loanAmount)
        # read database configuration
        params = databaseConfig()
        # connect to the PostgreSQL database
        conn = psycopg2.connect(**params)
        # create a new cursor
        cur = conn.cursor()

        # Check if the bank has enough money to lend.
        cur.execute("""SELECT balance from bank""")
        bankBalance = cur.fetchone()[0]
        if float(bankBalance) <= float(loanAmount):
            conn.commit()
            cur.close()
            return "false"
        
        # Get the account's balance.
        cur.execute("""SELECT balance FROM accounts WHERE accountID=(%s) AND userID=(%s)""", 
        (accountID, tokenDecrypt(token)['userID']))
        accountBalance = cur.fetchone()[0]

        # Add the loan amount to the account.
        accountBalance = float(loanAmount) + float(accountBalance)

        # Subtract the bank balance.
        bankBalance = float(bankBalance) - float(loanAmount)

        # Update the bank.
        cur.execute("""UPDATE bank SET balance=(%s)""", (bankBalance,))

        # Update the account.
        cur.execute("""UPDATE accounts 
            SET balance=(%s) WHERE accountID=(%s) 
            AND USERID=(%s)""",
            (accountBalance, accountID, tokenDecrypt(token)['userID']));
        
        # Add the loan to the loan logs.
        cur.execute("""INSERT INTO loans (accountID, amount) VALUES (%s, %s)""",
        (accountID, loanAmount))
      
        # commit the changes to the database
        conn.commit()
        # close communication with the database
        cur.close()

        # Return the account information.
        return "true"
    except (Exception, psycopg2.DatabaseError) as error:
        print(error)
    finally:
        if conn is not None:
            conn.close()
