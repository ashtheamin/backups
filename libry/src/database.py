import sqlite3
import os

def database_init():
    if os.path.exists("books.db") == False:
        con = sqlite3.connect('books.db')
        cur = con.cursor()

        cur.execute('''CREATE TABLE books (id text, title text, year text, description text)''')

        con.commit()
        con.close()

def database_book_load():
    if os.path.exists("books.db") == False:
        database_init()

    con = sqlite3.connect('books.db')
    cur = con.cursor()

    book_database = []
    for row in cur.execute('SELECT * FROM books'):
        book = {"id":"","title":"", "year":"", "description":""}
        book["id"] = row[0]
        book['title'] = row[1]
        book['year'] = row[2]
        book['description'] = row[3]
        book_database.append(book)

    con.commit()
    con.close()
    return book_database

def database_book_add(id, title, year, description):
    if os.path.exists("books.db") == False:
        database_init()
    
    con = sqlite3.connect('books.db')
    cur = con.cursor()

    cur.execute("INSERT INTO books VALUES (?, ?, ?, ?)", (id, title, year, description))

    con.commit()
    con.close()

def database_book_delete(id):
    if os.path.exists("books.db") == False:
        database_init()
    
    con = sqlite3.connect('books.db')
    cur = con.cursor()

    cur.execute("DELETE FROM books WHERE id=(?)", (id))

    con.commit()
    con.close()
