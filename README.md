# Лабораторная работа №5

## Задание

1. Создать `CMakeLists.txt` для библиотеки `banking`.
2. Создать модульные тесты для классов `Transaction` и `Account`.

   * Использовать mock-объекты.
   * Покрытие кода должно составлять 100%.
3. Настроить сборочную процедуру на Travis CI.
4. Настроить Coveralls.io.

---

## 1. Библиотека `banking`

В лабораторной работе используется исходный код библиотеки `banking`, содержащий классы `Account` и `Transaction`.

Структура библиотеки:

```text
banking/
├── Account.cpp
├── Account.h
├── CMakeLists.txt
├── Transaction.cpp
└── Transaction.h
```

Файл `banking/CMakeLists.txt`:

```cmake
add_library(banking
    Account.cpp
    Transaction.cpp
)

target_include_directories(banking PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
```

Команда `add_library` собирает файлы `Account.cpp` и `Transaction.cpp` в библиотеку `banking`.

---

## 2. Корневой `CMakeLists.txt`

В корневом `CMakeLists.txt` подключается библиотека `banking`, GoogleTest/GoogleMock и тесты.

```cmake
cmake_minimum_required(VERSION 3.10)

project(lab05 VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Build tests" ON)
option(ENABLE_COVERAGE "Enable coverage" ON)

if(ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(--coverage)
    add_link_options(--coverage)
endif()

add_subdirectory(banking)

if(BUILD_TESTS)
    enable_testing()

    add_subdirectory(third-party/gtest)
    add_subdirectory(tests)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(gtest PRIVATE -Wno-error=maybe-uninitialized)
endif()
```

GoogleTest подключён как git submodule:

```text
third-party/gtest
```

---

## 3. Модульные тесты `Account` и `Transaction`

Тесты находятся в файле:

```text
tests/bank_test.cpp
```

Для тестирования используются GoogleTest и GoogleMock.

### Mock-объекты

Для проверки виртуальных методов созданы mock-классы.

```cpp
class MockAccount : public Account {
 public:
  MockAccount(int id, int balance) : Account(id, balance) {}

  MOCK_METHOD0(Unlock, void());
};

class MockTransaction : public Transaction {
 public:
  MOCK_METHOD3(SaveToDataBase, void(Account& from, Account& to, int sum));
};
```

Пример использования mock-объекта `MockAccount`:

```cpp
TEST(AccountTest, MockObjectIsUsed) {
  NiceMock<MockAccount> account(1, 100);

  EXPECT_CALL(account, Unlock())
      .Times(1);

  account.Unlock();
}
```

Пример использования mock-объекта `MockTransaction`:

```cpp
TEST(TransactionTest, SuccessfulTransaction) {
  Account from(1, 500);
  Account to(2, 500);
  MockTransaction transaction;

  EXPECT_CALL(transaction, SaveToDataBase(_, _, 100))
      .Times(1);

  EXPECT_TRUE(transaction.Make(from, to, 100));

  EXPECT_EQ(from.GetBalance(), 500);
  EXPECT_EQ(to.GetBalance(), 499);
}
```

### Тесты класса `Account`

Проверяются:

* создание счёта и получение `id` и баланса;
* запрет изменения баланса без блокировки;
* изменение баланса после `Lock()`;
* исключение при повторном `Lock()`;
* работа `Unlock()`;
* использование mock-объекта.

Пример проверки запрета изменения баланса без блокировки:

```cpp
TEST(AccountTest, ChangeBalanceWithoutLockThrows) {
  Account account(1, 100);

  EXPECT_THROW(account.ChangeBalance(50), std::runtime_error);
}
```

Пример изменения баланса после блокировки:

```cpp
TEST(AccountTest, ChangeBalanceAfterLock) {
  Account account(1, 100);

  account.Lock();
  account.ChangeBalance(50);

  EXPECT_EQ(account.GetBalance(), 150);

  account.Unlock();
}
```

### Тесты класса `Transaction`

Проверяются:

* значение комиссии по умолчанию;
* изменение комиссии;
* перевод между счетами с одинаковым `id`;
* отрицательная сумма;
* слишком маленькая сумма;
* слишком большая комиссия;
* успешная транзакция;
* неуспешное списание и откат операции;
* настоящий вызов `SaveToDataBase()`.

Пример проверки одинакового `id`:

```cpp
TEST(TransactionTest, SameAccountThrows) {
  Account from(1, 500);
  Account to(1, 500);
  Transaction transaction;

  EXPECT_THROW(transaction.Make(from, to, 100), std::logic_error);
}
```

Пример проверки слишком маленькой суммы:

```cpp
TEST(TransactionTest, TooSmallSumThrows) {
  Account from(1, 500);
  Account to(2, 500);
  Transaction transaction;

  EXPECT_THROW(transaction.Make(from, to, 50), std::logic_error);
}
```

---

## 4. Сборка проекта

Конфигурация проекта:

```bash
cmake -S . -B build \
  -DBUILD_TESTS=ON \
  -DENABLE_COVERAGE=ON \
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5
```

Сборка:

```bash
cmake --build build -j2
```

Результат:

```text
[100%] Linking CXX executable check
[100%] Built target check
```

---

## 5. Запуск тестов

Запуск через CTest:

```bash
ctest --test-dir build --output-on-failure
```

Результат:

```text
Test project /home/nikita/lab05/build
    Start 1: banking_tests
1/1 Test #1: banking_tests ....................   Passed    0.01 sec

100% tests passed, 0 tests failed out of 1
```

Также тесты были запущены напрямую:

```bash
./build/tests/check
```

Результат:

```text
Running main() from gmock_main.cc
[==========] Running 15 tests from 2 test cases.
[----------] 6 tests from AccountTest
...
[----------] 9 tests from TransactionTest
...
[==========] 15 tests from 2 test cases ran.
[  PASSED  ] 15 tests.
```

Во время теста настоящего `SaveToDataBase()` получен вывод:

```text
1 send to 2 $100
Balance 1 is 500
Balance 2 is 499
```

Всего выполнено 15 тестов, все тесты завершились успешно.

---

## 6. Покрытие кода

Для проверки покрытия использована утилита `gcovr`.

Перед построением отчёта старые данные покрытия удаляются:

```bash
find build -name '*.gcda' -delete
```

После этого тесты запускаются заново:

```bash
ctest --test-dir build --output-on-failure
```

Команда построения отчёта:

```bash
gcovr \
  --root . \
  --filter 'banking/.*' \
  --exclude 'third-party/.*' \
  --exclude 'tests/.*' \
  --txt
```

Полученный результат:

```text
------------------------------------------------------------------------------
                           GCC Code Coverage Report
Directory: .
------------------------------------------------------------------------------
File                                       Lines    Exec  Cover   Missing
------------------------------------------------------------------------------
banking/Account.cpp                           13      13   100%
banking/Account.h                              1       1   100%
banking/Transaction.cpp                       33      33   100%
banking/Transaction.h                          2       2   100%
------------------------------------------------------------------------------
TOTAL                                         49      49   100%
------------------------------------------------------------------------------
```

Итог:

```text
49 / 49 строк = 100%
```

Требование о 100% покрытии исходного кода библиотеки `banking` выполнено.

---

## 7. Travis CI

Для автоматической сборки проекта создан файл `.travis.yml`.

```yaml
language: cpp

dist: focal

compiler:
  - gcc

before_install:
  - sudo apt-get update
  - sudo apt-get install -y cmake gcovr
  - pip3 install --user cpp-coveralls

script:
  - cmake -S . -B build -DBUILD_TESTS=ON -DENABLE_COVERAGE=ON -DCMAKE_POLICY_VERSION_MINIMUM=3.5
  - cmake --build build -j2
  - ctest --test-dir build --output-on-failure
  - ./build/tests/check
  - gcovr --root . --filter 'banking/.*' --exclude 'third-party/.*' --exclude 'tests/.*' --txt

after_success:
  - coveralls --root . -E ".*third-party.*" -E ".*tests.*" -E ".*CMakeFiles.*"
```

Travis CI выполняет:

1. установку необходимых зависимостей;
2. конфигурацию проекта через CMake;
3. сборку библиотеки и тестов;
4. запуск CTest;
5. запуск тестового приложения;
6. проверку покрытия;
7. отправку покрытия в Coveralls после успешной сборки.

---

## 8. Coveralls.io

Для отправки статистики покрытия используется `cpp-coveralls`.

После успешной сборки выполняется команда:

```bash
coveralls --root . \
  -E ".*third-party.*" \
  -E ".*tests.*" \
  -E ".*CMakeFiles.*"
```

Из статистики покрытия исключаются:

* GoogleTest и GoogleMock;
* тестовые файлы;
* служебные файлы CMake.

Благодаря этому Coveralls должен учитывать непосредственно исходный код библиотеки `banking`.

---

## Итог

В лабораторной работе:

* создан `CMakeLists.txt` для библиотеки `banking`;
* написаны модульные тесты для `Account`;
* написаны модульные тесты для `Transaction`;
* использованы mock-объекты;
* успешно пройдены 15 тестов;
* получено покрытие 100% — 49 из 49 строк;
* создан файл конфигурации Travis CI;
* настроена отправка покрытия в Coveralls.
