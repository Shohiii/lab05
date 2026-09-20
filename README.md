# Лабораторная работа №5

репозиторий: https://github.com/Shohiii/lab05

условие лабораторной работы: https://github.com/tp-labs/lab05

## Задание

1. Создать `CMakeLists.txt` для библиотеки `banking`.
2. Создать модульные тесты для классов `Transaction` и `Account`.
   * Использовать mock-объекты.
   * Покрытие кода должно составлять 100%.
3. Настроить процедуру автоматической сборки и тестирования в CI.
4. Настроить Coveralls.io.

> Вместо Travis CI используется GitHub Actions, как в принятом варианте лабораторной работы.

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

---

## 2. Подключение GoogleTest как git submodule

GoogleTest/GoogleMock подключён как git submodule:

```text
third-party/gtest
```

Проверка:

```bash
git submodule status
```

Используется GoogleTest `release-1.8.1`.

---

## 3. Корневой `CMakeLists.txt`

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

Файл `tests/CMakeLists.txt`:

```cmake
add_executable(check
    bank_test.cpp
)

target_link_libraries(check
    banking
    gmock_main
)

add_test(NAME banking_tests COMMAND check)
```

---

## 4. Модульные тесты `Account` и `Transaction`

Тесты находятся в файле:

```text
tests/bank_test.cpp
```

Для тестирования используются GoogleTest и GoogleMock.

### Mock-объекты

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

Пример использования `MockAccount`:

```cpp
TEST(AccountTest, MockObjectIsUsed) {
  NiceMock<MockAccount> account(1, 100);

  EXPECT_CALL(account, Unlock())
      .Times(1);

  account.Unlock();
}
```

Пример использования `MockTransaction`:

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

Проверяются:
* создание `Account`;
* изменение баланса только после блокировки;
* повторный `Lock()`;
* `Unlock()`;
* комиссия `Transaction`;
* одинаковые `id`;
* отрицательная и слишком маленькая сумма;
* слишком большая комиссия;
* успешная транзакция;
* откат при неуспешном списании;
* вызов `SaveToDataBase()`.

---

## 5. Сборка проекта

```bash
cmake -S . -B build \
  -DBUILD_TESTS=ON \
  -DENABLE_COVERAGE=ON \
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5
```

```bash
cmake --build build -j2
```

Результат:

```text
[100%] Linking CXX executable check
[100%] Built target check
```

---

## 6. Запуск тестов

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

Прямой запуск:

```bash
./build/tests/check
```

Результат:

```text
[==========] Running 15 tests from 2 test cases.
[----------] 6 tests from AccountTest
...
[----------] 9 tests from TransactionTest
...
[==========] 15 tests from 2 test cases ran.
[  PASSED  ] 15 tests.
```

Во время вызова настоящего `SaveToDataBase()`:

```text
1 send to 2 $100
Balance 1 is 500
Balance 2 is 499
```

---

## 7. Покрытие кода

Перед построением отчёта:

```bash
find build -name '*.gcda' -delete
ctest --test-dir build --output-on-failure
```

Проверка покрытия:

```bash
gcovr \
  --root . \
  --filter 'banking/.*' \
  --exclude 'third-party/.*' \
  --exclude 'tests/.*' \
  --txt
```

Результат:

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

Итог: **49 из 49 строк, 100%**.

---

## 8. CI: GitHub Actions

Вместо Travis CI используется GitHub Actions.

Файл:

```text
.github/workflows/tests.yml
```

Workflow выполняет:
1. checkout репозитория вместе с submodule;
2. установку CMake, GCC и `gcovr`;
3. конфигурацию проекта;
4. сборку;
5. запуск тестов;
6. проверку покрытия.

После push workflow `Lab05 Tests and Coverage` успешно выполнился в разделе **Actions** репозитория.

---

## 9. Coveralls.io

Для Coveralls в GitHub Actions создаётся LCOV-файл:

```bash
gcovr \
  --root . \
  --filter 'banking/.*' \
  --exclude 'third-party/.*' \
  --exclude 'tests/.*' \
  --lcov coverage.info
```

После этого файл отправляется в Coveralls действием:

```yaml
- name: Upload coverage to Coveralls
  uses: coverallsapp/github-action@v2
  with:
    github-token: ${{ secrets.GITHUB_TOKEN }}
    file: coverage.info
    format: lcov
```

---

## Итог

В лабораторной работе:

* создан `CMakeLists.txt` для библиотеки `banking`;
* GoogleTest/GoogleMock подключён как git submodule;
* написаны тесты для `Account` и `Transaction`;
* использованы mock-объекты;
* успешно пройдены 15 тестов;
* получено покрытие 100% — 49 из 49 строк;
* вместо Travis CI используется GitHub Actions;
* workflow GitHub Actions успешно выполняется;
* предусмотрена отправка покрытия в Coveralls.
