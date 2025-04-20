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