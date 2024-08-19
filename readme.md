# Тестовое задание для C++ разработчиков

![NTPro Logo](NTPro-Logo.png)

## Сборка
---

Тестирование происходило на WSL2 с Ubuntu.
Для скачивания boost используем conan (тестировалось на версии conan 1.64.0):
```
pip install --force-reinstall -v "conan==1.64.0"
```

Порядок сборки:
```
mkdir build  && cd build
conan install .. --build=missing -s build_type=Release -s compiler.libcxx=libstdc++11
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

## Использование 
---
Всё сказанное ниже выполняется из папки `build`

Для запуска тестов:
```
./bin/BoostTest --report_level=short
```
Для запуска сервера:
```
./bin/Server
```
Для запуска клиента (**Должен быть запущен сервер!**):
```
./bin/Client
```

## Реализованные команды 
---
## Registration
При запуске клиента у пользователя запросят `имя` и `пароль`.
Если под `имя` никто не пытался зайти на сервер, сервер зарегистрирует нового пользователя и выдаст клиенту `ID`.
Если `имя` зарегистривано на сервере, то сервер сопоставит введённый `пароль` и сохранённый на сервере.
При совпадении сервер выдаст клиенту `ID`, иначе отклонит авторизацию.

После регистрации пользователь клиента попадает в меню и может выполнять запросы к серверу.

---

### 1) Sell и 2) Buy
Команды отправляют запрос серверу на создание заявок на покупку/продажу.

При вызове запросит у клиента количество `долларов` и `рублей` для сделки.
Если введённое количество `долларов` или `рублей` будет меньше или равно рулю сервер не примет заявку и известит клиент.

---

### 3) Check balance
Возвращает текущий баланс `долларов` и `рублей` на счету.

---

### 4) My active transactions
Возвращает список текущих активных заявок на покупку/продажу от данного пользователя.

---

### 5) Change transaction
Отправляет запрос на изменение заявки.

При вызове запросит у клиента `ID` заявки для изменения.
Затем запросит у клиента новое количество `долларов` и `рублей`.

Сервер проверит активна ли заявка с `ID`, затем проверит принадлежит ли эта заявка клиенту.
Если всё в порядке и новые данные в завке валидны (введённое количество `долларов` и `рублей` больше нуля).
То сервер поменяет данные в заявке и известит пользователя.

---

### 6) All active transactions
Возвращает список текущих активных заявок на покупку/продажу от всех пользователей.

---


## 📈 Легенда
---

Ты стал новым руководителем отдела биржи. Поздравляем! 🎉

Но выяснилось, что в твоём отделе уже несколько десятков лет торговля ведется по старинке! Все измучены миллионами звонков от трейдеров, выставляющих заявки по телефону. 🤳

Ты хочешь всё это автоматизировать, чтобы клиенты могли торговать через компьютер. Тогда все могли бы сидеть в тишине и обрабатывать их заявки ☺️

К счастью, у тебя есть для этого всё — Linux, C++ и полный гитхаб различных библиотек. 

## 🔮 Что необходимо сделать
---

Необходимо написать простую биржу с клиент-серверной архитектурой. 
Биржа будет торговать только долларами (USD) за рубли (RUB).

В случае, если цена на покупку и цена на продажу у нескольких клиентов пересекается — нужно заключать сделки между ними. 
В этом случае купленный объём будет зачисляться на баланс клиентам.

### ℹ️ Функциональные требования
---

**Сервер**

- Поддерживает подключения нескольких клиентов одновременно.
- Принимает заявки на покупку или продажу долларов за рубли от разных клиентов.
- Даёт возможность просмотра баланса клиента.

**Клиент**

- Подключается к серверу и реализует все его возможности.

**Торговая логика**

- Торговая заявка содержит объём, цену и сторону (покупка/продажа).
- Если две заявки пересекаются по цене — для сделки выбирается цена более ранней заявки.
- Если заявка пересекается с несколькими заявками по другой стороне — приоритет в исполнении отдаётся заявкам с 
максимальной ценой для заявок на покупку и с минимальной ценой для заявок на продажу.
- Возможно частичное исполнение заявки. (см. пример)
- Торговая заявка активна до тех пор, пока по ней есть активный объём.
- Баланс клиента не ограничен — он может торговать в минус.

📝 **Пример:**

- Пользователь 1 (П1) выставил заявку на покупку **10** USD за RUB по цене 62.
- Пользователь 2 (П2) выставил заявку на покупку **20** USD за RUB по цене 63.
- Пользователь 3 (П3) выставил заявку на продажу **50** USD за RUB по цене 61.
- Сервер сматчил эти заявки между собой. Получилось две сделки:
    - На **20**$ по цене 63 между П2 и П3.
    - На **10**$ по цене 62 между П1 и П3.
- Теперь на балансе пользователей:
    - Пользователь 1: **10** USD, **-620** RUB.
    - Пользователь 2: **20** USD, **-1260** RUB.
    - Пользователь 3: **-30** USD, **1880** RUB.
- Торговая заявка пользователя 3 (частично исполненная на **30$**) осталась активной на **20$**.

> ⚠️ Важно! Мы исполняемся в таком порядке неслучайно: начали с заявки П2, поскольку её цена больше, чем у П1.

🎈 **Бонусы! Опционально реализуйте:**

> Внимание! Бонусы будут учитываться нами **только** при условии, если основная часть будет сделана **ИДЕАЛЬНО**.
> 
> Важнее сделать качественно основную часть, чем много бонусов.

- Просмотр активных заявок. 📋
- Просмотр совершённых сделок. 🛂
- Просмотр истории котировок. 📈
- Получение отчёта о совершённой сделке в момент её заключения всеми сторонами-участниками. 🤼
- Отмена активных заявок. 🚫
- Сохранение истории заявок и сделок в базу данных (например, PostgreSQL). 💽
- Авторизация клиента с паролем. 🔑
- GUI к бирже на QT. 🤯

## 🧑‍💻 Требования по реализации
---

- Клиент может представлять собой обычное консольное приложение.
- Все основные торговые сценарии на сервере должны быть покрыты тестами: 
    - Обработка заявки без совершения сделки
    - Частичное исполнение
    - Исполнение с несколькими заявками 
    - ...  
- Проект должен быть написан с использованием C++17/20.
- Проект должен собираться с использованием CMake.
- Разрешается использование сторонних библиотек.
- Описание, инструкция по сборке и инструкция по использованию проекта должны быть в файле **README.md**
- Проект должен быть выполнен в git-репозитории, ссылку на который необходимо предоставить.

## 🔦 Что мы оцениваем
---

Для нас важно:

- Все требования по функциональности учтены
- Есть все необходимые тесты
- Код написан и оформлен качественно

## 🤝 Каркас тестового задания
---

Для упрощения задачи мы **приготовили каркас** для нашего тестового задания, чтобы вам не пришлось с нуля писать сеть и взаимодействие с сервером.
Он призван сэкономить ваше время, а не навязать архитектуру реализации. 
Его можно не использовать, если вы не хотите.

Для его создания мы использовали библиотеки boost::asio и nlohmann::json.

Пример специально был создан таким образом, чтобы для выполнения тестового задания его архитектуру пришлось дорабатывать.
Вы можете смело переписывать все функции и рефакторить этот код.
Все наши ограничения прописаны в требованиях к тестовому заданию.

В примере реализована "регистрация" клиента у сервера и один единственный запрос - на получение имени при регистрации.

Удачи в разработке!
