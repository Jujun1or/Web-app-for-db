async function loadBooks() {
    try {
        const response = await fetch('http://localhost:8080/books');
        const books = await response.json();
        
        let html = '<ul>';
        books.forEach(book => {
            html += `<li>${book.title} (${book.price} руб.)</li>`;
        });
        html += '</ul>';
        
        document.getElementById('bookList').innerHTML = html;
    } catch (error) {
        console.error('Ошибка:', error);
    }
}

async function createUser() {
    const name = document.getElementById('userName').value;
    const date = document.getElementById('userDate').value;
    
    if (!name) {
        alert('Введите имя пользователя');
        return;
    }

    if (!date) {
        alert('Введите дату регистрации пользователя');
        return;
    }


    try {
        const response = await fetch('http://localhost:8080/users', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify({
                name: name,
                date: date || new Date().toISOString().split('T')[0]
            })
        });
        
        const result = await response.json();
        document.getElementById('userStatus').innerHTML = 
            `Статус: ${result.status}<br>Сообщение: ${result.message}`;
        
        if (result.status === 'success') {
            document.getElementById('userName').value = '';
            document.getElementById('userDate').value = '';
        }
    } catch (error) {
        console.error('Ошибка:', error);
    }
}

async function topUpFund() {
    const summ = parseInt(document.getElementById('summ').value);
    const date = document.getElementById('topUpDate').value;
    
    if (!summ) {
        alert('Введите сумму');
        return;
    }

    if (!date) {
        alert('Введите дату пополнения');
        return;
    }


    try {
        const response = await fetch('http://localhost:8080/fond_top_up', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify({
                summ: summ,
                date: date || new Date().toISOString().split('T')[0]
            })
        });
        
        const result = await response.json();
        document.getElementById('topUpStatus').innerHTML = 
            `Статус: ${result.status}<br>Сообщение: ${result.message}`;
        
        if (result.status === 'success') {
            document.getElementById('summ').value = '';
            document.getElementById('topUpDate').value = '';
        }
    } catch (error) {
        console.error('Ошибка:', error);
    }
}


async function returnBook() {
    const userId = document.getElementById('returnUserId').value;
    const bookId = document.getElementById('returnBookId').value;
    const date = document.getElementById('returnDate').value;

    if(!userId || !bookId || !date) {
        alert('Заполните все поля');
        return;
    }

    try {
        const response = await fetch('http://localhost:8080/return-book', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify({
                user_id: parseInt(userId),
                book_id: parseInt(bookId),
                date: date
            })
        });
        
        const result = await response.json();
        document.getElementById('returnStatus').innerHTML = 
            `Статус: ${result.status}<br>Сообщение: ${result.message}`;
        
        if(result.status === 'success') {
            document.getElementById('returnUserId').value = '';
            document.getElementById('returnBookId').value = '';
        }
    } catch(error) {
        console.error('Ошибка:', error);
    }
}

// Утеря книги
async function reportLostBook() {
    const userId = document.getElementById('lostUserId').value;
    const bookId = document.getElementById('lostBookId').value;
    const date = document.getElementById('lostDate').value;

    if(!userId || !bookId || !date) {
        alert('Заполните все поля');
        return;
    }

    try {
        const response = await fetch('http://localhost:8080/lost-book', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify({
                user_id: parseInt(userId),
                book_id: parseInt(bookId),
                date: date
            })
        });
        
        const result = await response.json();
        document.getElementById('lostStatus').innerHTML = 
            `Статус: ${result.status}<br>Сообщение: ${result.message}`;
        
        if(result.status === 'success') {
            document.getElementById('lostUserId').value = '';
            document.getElementById('lostBookId').value = '';
        }
    } catch(error) {
        console.error('Ошибка:', error);
    }
}

async function issueBook() {
    const userId = document.getElementById('issueUserId').value;
    const bookId = document.getElementById('issueBookId').value;
    const date = document.getElementById('issueDate').value;

    if(!validateInputs(userId, bookId, date)) return;

    try {
        const response = await fetch('http://localhost:8080/issue-book', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                user_id: Number(userId),
                book_id: Number(bookId),
                date: date
            })
        });
        
        handleResponse(response, 'issueStatus');
    } catch(error) {
        showError('issueStatus', error);
    }
}

// Продление книги 
async function extendBook() {
    const userId = document.getElementById('extendUserId').value;
    const bookId = document.getElementById('extendBookId').value;
    const extensionDays = document.getElementById('extensionDays').value;

    if(!userId || !bookId || !extensionDays) {
        alert('Заполните все обязательные поля');
        return;
    }

    if(extensionDays <= 0) {
        alert('Количество дней продления должно быть положительным числом');
        return;
    }

    try {
        const response = await fetch('http://localhost:8080/extend-book', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                user_id: Number(userId),
                book_id: Number(bookId),
                extensionTime: Number(extensionDays)
            })
        });
        
        handleResponse(response, 'extendStatus');
    } catch(error) {
        showError('extendStatus', error);
    }
}

// Общие функции
function validateInputs(userId, bookId, date) {
    if(!userId || !bookId || !date) {
        alert('Заполните все обязательные поля');
        return false;
    }
    return true;
}

async function handleResponse(response, elementId) {
    const result = await response.json();
    const statusDiv = document.getElementById(elementId);
    statusDiv.innerHTML = `Статус: ${result.status}<br>${result.message}`;
    statusDiv.setAttribute('data-status', result.status);
    
    if(result.status === 'success') {
        clearInputs(elementId);
    }
}

function clearInputs(elementId) {
    const prefix = elementId.replace('Status', '');
    document.getElementById(`${prefix}UserId`).value = '';
    document.getElementById(`${prefix}BookId`).value = '';
    document.getElementById(`${prefix}Date`).value = '';
}

function showError(elementId, error) {
    const statusDiv = document.getElementById(elementId);
    statusDiv.innerHTML = `Ошибка: ${error.message}`;
    statusDiv.setAttribute('data-status', 'error');
}

async function performSearch() {
    const query = document.getElementById('searchQuery').value;
    const searchType = document.querySelector('input[name="searchType"]:checked').value;
    
    if (!query) {
        alert('Введите поисковый запрос');
        return;
    }

    try {
        const response = await fetch('http://localhost:8080/search', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({
                query: query,
                search_type: searchType
            })
        });
        
        const results = await response.json();
        updateResultsTable(results);
    } catch(error) {
        console.error('Ошибка поиска:', error);
    }
}

function updateResultsTable(results) {
    const tbody = document.getElementById('resultsBody');
    tbody.innerHTML = '';
    
    results.forEach(book => {
        const row = document.createElement('tr');
        
        const idCell = document.createElement('td');
        idCell.textContent = book.id;
        row.appendChild(idCell);
        
        const titleCell = document.createElement('td');
        titleCell.textContent = book.title;
        row.appendChild(titleCell);
        
        const authorsCell = document.createElement('td');
        authorsCell.textContent = book.authors.join(', ');
        row.appendChild(authorsCell);
        
        tbody.appendChild(row);
    });
}