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


async function issueBook() {
    const userName = document.getElementById('issueUserName').value;
    const bookId = document.getElementById('issueBookId').value;
    const date = document.getElementById('issueDate').value;

    if(!userName || !bookId || !date) {
        alert('Заполните все поля');
        return;
    }

    try {
        const response = await fetch('http://localhost:8080/issue-book', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                user_name: userName,
                book_id: Number(bookId),
                date: date
            })
        });
        
        handleResponse(response, 'issueStatus');
    } catch(error) {
        showError('issueStatus', error);
    }
}

async function extendBook() {
    const userName = document.getElementById('extendUserName').value;
    const bookId = document.getElementById('extendBookId').value;
    const extensionDays = document.getElementById('extensionDays').value;

    if(!userName || !bookId || !extensionDays) {
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
                user_name: userName,
                book_id: Number(bookId),
                extensionTime: Number(extensionDays)
            })
        });
        
        handleResponse(response, 'extendStatus');
    } catch(error) {
        showError('extendStatus', error);
    }
}

async function returnBook() {
    const userName = document.getElementById('returnUserName').value;
    const bookId = document.getElementById('returnBookId').value;
    const date = document.getElementById('returnDate').value;

    if(!userName || !bookId || !date) {
        alert('Заполните все поля');
        return;
    }

    try {
        const response = await fetch('http://localhost:8080/return-book', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                user_name: userName,
                book_id: Number(bookId),
                date: date
            })
        });
        
        handleResponse(response, 'returnStatus');
    } catch(error) {
        showError('returnStatus', error);
    }
}

async function reportLostBook() {
    const userName = document.getElementById('lostUserName').value;
    const bookId = document.getElementById('lostBookId').value;
    const date = document.getElementById('lostDate').value;

    if(!userName || !bookId || !date) {
        alert('Заполните все поля');
        return;
    }

    try {
        const response = await fetch('http://localhost:8080/lost-book', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                user_name: userName,
                book_id: Number(bookId),
                date: date
            })
        });
        
        handleResponse(response, 'lostStatus');
    } catch(error) {
        showError('lostStatus', error);
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

async function loadOverdue() {
    try {
        const response = await fetch('/overdue');
        const result = await response.json();
        
        if (!response.ok) {
            throw new Error(result.message || 'Ошибка сервера');
        }
        
        const tbody = document.getElementById('overdueBody');
        tbody.innerHTML = '';
        
        if (result.data && Array.isArray(result.data)) {
            result.data.forEach(loan => {
                const row = document.createElement('tr');
                
                // Добавляем класс для штрафных записей
                if (loan.days_overdue > 365) {
                    row.classList.add('fine-row');
                }
                
                row.innerHTML = `
                    <td>${loan.user_name}</td>
                    <td>${loan.book_title}</td>
                    <td>${loan.days_overdue}</td>
                    <td>
                        <button onclick="generateLetter(${loan.bu_id})">
                            ${loan.days_overdue > 365 ? 'Штрафная квитанция' : 'Письмо-напоминание'}
                        </button>
                    </td>
                `;
                tbody.appendChild(row);
            });
        }
    } catch(error) {
        console.error('Ошибка:', error.message);
        alert('Не удалось загрузить данные: ' + error.message);
    }
}

async function generateLetter(bu_id) {
    try {
        const response = await fetch('/generate-letter', {
            method: 'POST',
            headers: {'Content-Type': 'application/json'},
            body: JSON.stringify({ bu_id: bu_id })
        });
        
        const result = await response.json();
        
        const modal = document.getElementById('letterModal');
        const content = document.getElementById('letterContent');
        
        let buttons = '';
        if (result.type === 'fine') {
            buttons = `
                <button onclick="payFine(${bu_id})">Оплатить штраф</button>
            `;
        } else {
            buttons = `<button onclick="closeModal()">Закрыть</button>`;
        }
        
        content.innerHTML = `
            <h3>${result.type === 'fine' ? 'Штрафная квитанция' : 'Уведомление'}</h3>
            <div class="letter-text">${result.letter_text.replace(/\n/g, '<br>')}</div>
            <div class="button-group">${buttons}</div>
        `;
        
        modal.style.display = 'block';
    } catch(error) {
        console.error('Ошибка:', error);
        alert('Ошибка: ' + error.message);
    }
}

async function payFine(bu_id) {
    try {
        const response = await fetch('/pay-fine', {
            method: 'POST',
            headers: {'Content-Type': 'application/json'},
            body: JSON.stringify({ bu_id: bu_id })
        });
        
        const result = await response.json();
        
        if (result.status === 'success') {
            alert('Штраф успешно оплачен! Книга списана.');
            closeModal();
            loadOverdue(); // Обновляем список
        } else {
            throw new Error(result.error || 'Ошибка оплаты');
        }
    } catch(error) {
        console.error('Ошибка:', error);
        alert('Ошибка оплаты: ' + error.message);
    }
}

function closeModal() {
    document.getElementById('letterModal').style.display = 'none';
}

async function loadPopularBooks() {
    const start = document.getElementById('startDate').value;
    const end = document.getElementById('endDate').value;
    
    if(!start || !end) {
        alert('Выберите период');
        return;
    }

    try {
        const response = await fetch(`/reports/popular?start_date=${start}&end_date=${end}`);
        const data = await response.json();
        
        const tbody = document.getElementById('popularBooks');
        tbody.innerHTML = '';
        
        data.forEach(book => {
            const row = document.createElement('tr');
            row.innerHTML = `
                <td>${book.rank}</td>
                <td>${book.title}</td>
                <td>${book.authors.join(', ')}</td>
                <td>${book.loans_count}</td>
            `;
            tbody.appendChild(row);
        });
    } catch(error) {
        console.error('Ошибка:', error);
    }
}

async function loadReadersStats() {
    const year = document.getElementById('yearInput').value;
    
    try {
        const response = await fetch(`/reports/readers?year=${year}`);
        const data = await response.json();
        
        const tbody = document.getElementById('readersStats');
        tbody.innerHTML = '';
        
        data.forEach(reader => {
            const row = document.createElement('tr');
            row.innerHTML = `
                <td>${reader.rank}</td>
                <td>${reader.name}</td>
                <td>${reader.books_count}</td>
            `;
            tbody.appendChild(row);
        });
    } catch(error) {
        console.error('Ошибка:', error);
    }
}

async function loadFinancialReport() {
    try {
        const monthsNames = [
            'Январь', 'Февраль', 'Март', 'Апрель',
            'Май', 'Июнь', 'Июль', 'Август',
            'Сентябрь', 'Октябрь', 'Ноябрь', 'Декабрь'
        ];

        const yearInput = document.getElementById('reportYear');
        const year = yearInput.value;
        
        if (!year) {
            alert('Пожалуйста, введите год');
            return;
        }

        const response = await fetch(`/reports/financial?year=${year}`);
        const report = await response.json();
        
        const tbody = document.getElementById('reportBody');
        tbody.innerHTML = '';

        let quarter = 1;
        let qExpenses = 0, qIncome = 0, qTotal = 0;
        let yExpenses = 0, yIncome = 0, yTotal = 0;

        report.months.forEach((monthData, index) => {
            const monthName = monthsNames[monthData.month - 1];
            
            if (index % 3 === 0) {
                tbody.innerHTML += `
                <tr class="quarter-header">
                    <td colspan="4">Квартал ${quarter}</td>
                </tr>`;
            }

            tbody.innerHTML += `
            <tr>
                <td>${monthName}</td>
                <td>${monthData.expenses}</td>
                <td>${monthData.income}</td>
                <td>${monthData.income - monthData.expenses}</td>
            </tr>`;

            qExpenses += monthData.expenses;
            qIncome += monthData.income;
            qTotal += monthData.income - monthData.expenses;

            if ((index + 1) % 3 === 0) {
                tbody.innerHTML += `
                <tr class="quarter-total">
                    <td>Итого за квартал ${quarter}</td>
                    <td>${qExpenses}</td>
                    <td>${qIncome}</td>
                    <td>${qTotal}</td>
                </tr>`;

                yExpenses += qExpenses;
                yIncome += qIncome;
                yTotal += qTotal;

                qExpenses = qIncome = qTotal = 0;
                quarter++;
            }
        });

        tbody.innerHTML += `
        <tr class="year-total">
            <td>Годовой итог</td>
            <td>${yExpenses}</td>
            <td>${yIncome}</td>
            <td>${yTotal}</td>
        </tr>
        <tr class="total-assets">
            <td colspan="3">Общая стоимость фондов</td>
            <td>${report.total_assets + yTotal}</td>
        </tr>`;
        
    } catch(error) {
        console.error('Ошибка:', error);
        alert('Ошибка загрузки отчета: ' + error.message);
    }
}

async function checkInactiveUsers() {
    try {
        const response = await fetch('/deactivate-users', {
            method: 'POST',
            headers: {'Content-Type': 'application/json'}
        });
        
        const result = await response.json();
        const tbody = document.getElementById('deactivatedUsers');
        tbody.innerHTML = '';
        
        if (result.status === 'success') {
            result.deactivated_users.forEach(user => {
                const row = document.createElement('tr');
                row.innerHTML = `
                    <td>${user.user_id}</td>
                    <td>${user.name}</td>
                    <td>Отчислен</td>
                `;
                tbody.appendChild(row);
            });
            
            if (result.deactivated_users.length === 0) {
                tbody.innerHTML = '<tr><td colspan="3">Нет пользователей для отчисления</td></tr>';
            }
        } else {
            alert('Ошибка: ' + result.message);
        }
    } catch(error) {
        console.error('Ошибка:', error);
        alert('Ошибка выполнения операции');
    }
}