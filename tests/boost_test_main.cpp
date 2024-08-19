#define BOOST_TEST_MODULE leap year application tests
#include <boost/test/unit_test.hpp>

#include "../src/Server.h"

struct TestFixture
{
  server::Core core_instance;

  TestFixture()
  : core_instance()
  {}

  ~TestFixture() = default;
};

BOOST_FIXTURE_TEST_SUITE(TestCore, TestFixture)

// Регистрация
BOOST_AUTO_TEST_CASE(Registration)
{
  std::string str = R"({"Message":{"Password":"23","UserId":0,"Username":"test"},"ReqType":"REG"})";
  nlohmann::json json = nlohmann::json::parse(str);
  BOOST_REQUIRE_EQUAL(R"({"Code":"OK","Id":0,"Message":"New user successfully registered"})", core_instance.HandleRequest(json));
}

// Регистрация нескольких пользователей
BOOST_AUTO_TEST_CASE(MultipleRegistrations)
{
  std::string str = R"({"Message":{"Password":"23","UserId":0,"Username":"test"},"ReqType":"REG"})";
  nlohmann::json json = nlohmann::json::parse(str);
  BOOST_REQUIRE_EQUAL(R"({"Code":"OK","Id":0,"Message":"New user successfully registered"})", core_instance.HandleRequest(json));

  str = R"({"Message":{"Password":"34","UserId":0,"Username":"test1"},"ReqType":"REG"})";
  json = nlohmann::json::parse(str);
  BOOST_REQUIRE_EQUAL(R"({"Code":"OK","Id":1,"Message":"New user successfully registered"})", core_instance.HandleRequest(json));

  str = R"({"Message":{"Password":"56","UserId":0,"Username":"test2"},"ReqType":"REG"})";
  json = nlohmann::json::parse(str);
  BOOST_REQUIRE_EQUAL(R"({"Code":"OK","Id":2,"Message":"New user successfully registered"})", core_instance.HandleRequest(json));
}

// Вход существующего пользователя
BOOST_AUTO_TEST_CASE(ReEntry)
{
  std::string str = R"({"Message":{"Password":"23","UserId":0,"Username":"test"},"ReqType":"REG"})";
  nlohmann::json json = nlohmann::json::parse(str);

  core_instance.HandleRequest(json);
  str = R"({"Message":{"Password":"23","UserId":0,"Username":"test"},"ReqType":"REG"})";
  json = nlohmann::json::parse(str);
  BOOST_REQUIRE_EQUAL(R"({"Code":"OK","Id":0,"Message":"Authentication successfully completed"})", core_instance.HandleRequest(json));
}

// Неправильный пароль существующего пользователя
BOOST_AUTO_TEST_CASE(WrongPassword)
{
  std::string str = R"({"Message":{"Password":"23","UserId":0,"Username":"test"},"ReqType":"REG"})";
  nlohmann::json json = nlohmann::json::parse(str);

  core_instance.HandleRequest(json);
  str = R"({"Message":{"Password":"22","UserId":0,"Username":"test"},"ReqType":"REG"})";
  json = nlohmann::json::parse(str);
  BOOST_REQUIRE_EQUAL(R"({"Code":"ERROR","Message":"Error! Wrong username or password"})", core_instance.HandleRequest(json));
}

// Пустое имя или пароль
BOOST_AUTO_TEST_CASE(EmptyNameOrPass)
{
  std::string str = R"({"Message":{"Password":"23","UserId":0,"Username":""},"ReqType":"REG"})";
  nlohmann::json json = nlohmann::json::parse(str);
  BOOST_REQUIRE_EQUAL(R"({"Code":"ERROR","Message":"Error! Empty name or password"})", core_instance.HandleRequest(json));

  str = R"({"Message":{"Password":"","UserId":0,"Username":"test1"},"ReqType":"REG"})";
  json = nlohmann::json::parse(str);
  BOOST_REQUIRE_EQUAL(R"({"Code":"ERROR","Message":"Error! Empty name or password"})", core_instance.HandleRequest(json));
}

// Набор тестов для добавления странзакций SELL
BOOST_AUTO_TEST_CASE(AddSELLTransaction) {
  // Регистрируем пользователя
  std::string str = R"({"Message":{"Password":"23","UserId":0,"Username":"test"},"ReqType":"REG"})";
  nlohmann::json json = nlohmann::json::parse(str);
  core_instance.HandleRequest(json);

  // Добавляем валидную сделку
  str = R"({"Message":{"Dollars":10,"Rubles":10,"UserId":0},"ReqType":"SELL"})";
  json = nlohmann::json::parse(str);
  BOOST_REQUIRE_EQUAL(R"({"Code":"OK","Message":"Transaction SELL 10 dollars for 10 rubles accepted"})", core_instance.HandleRequest(json));

  // Добавляем НЕ валидную сделку
  str = R"({"Message":{"Dollars":0,"Rubles":10,"UserId":0},"ReqType":"SELL"})";
  json = nlohmann::json::parse(str);
  BOOST_REQUIRE_EQUAL(R"({"Code":"ERROR","Message":"Error! Transaction is not valid"})", core_instance.HandleRequest(json));

  // Добавляем НЕ валидную сделку
  str = R"({"Message":{"Dollars":10,"Rubles":0,"UserId":0},"ReqType":"SELL"})";
  json = nlohmann::json::parse(str);
  BOOST_REQUIRE_EQUAL(R"({"Code":"ERROR","Message":"Error! Transaction is not valid"})", core_instance.HandleRequest(json));
}

// Набор тестов для добавления странзакций BUY
BOOST_AUTO_TEST_CASE(AddBUYTransaction) {
  // Регистрируем пользователя
  std::string str = R"({"Message":{"Password":"23","UserId":0,"Username":"test"},"ReqType":"REG"})";
  nlohmann::json json = nlohmann::json::parse(str);
  core_instance.HandleRequest(json);
  
  // Добавляем валидную сделку
  str = R"({"Message":{"Dollars":10,"Rubles":10,"UserId":0},"ReqType":"BUY"})";
  json = nlohmann::json::parse(str);
  BOOST_REQUIRE_EQUAL(R"({"Code":"OK","Message":"Transaction BUY 10 dollars for 10 rubles accepted"})", core_instance.HandleRequest(json));
  
  // Добавляем НЕ валидную сделку
  str = R"({"Message":{"Dollars":0,"Rubles":10,"UserId":0},"ReqType":"BUY"})";
  json = nlohmann::json::parse(str);
  BOOST_REQUIRE_EQUAL(R"({"Code":"ERROR","Message":"Error! Transaction is not valid"})", core_instance.HandleRequest(json));
  
  // Добавляем НЕ валидную сделку
  str = R"({"Message":{"Dollars":10,"Rubles":0,"UserId":0},"ReqType":"BUY"})";
  json = nlohmann::json::parse(str);
  BOOST_REQUIRE_EQUAL(R"({"Code":"ERROR","Message":"Error! Transaction is not valid"})", core_instance.HandleRequest(json));
}

// Тест-кейс полностью закрытой сделки
BOOST_AUTO_TEST_CASE(OneTransaction) {
  // Регистрируем пользователя {ID:0}
  std::string str = R"({"Message":{"Password":"23","UserId":0,"Username":"test"},"ReqType":"REG"})";
  nlohmann::json json = nlohmann::json::parse(str);
  core_instance.HandleRequest(json);

  // Регистрируем пользователя {ID:1}
  str = R"({"Message":{"Password":"32","UserId":0,"Username":"test1"},"ReqType":"REG"})";
  json = nlohmann::json::parse(str);
  core_instance.HandleRequest(json);

  // {ID:1} отправляет запрос на покупку
  str = R"({"Message":{"Dollars":10,"Rubles":10,"UserId":1},"ReqType":"BUY"})";
  json = nlohmann::json::parse(str);
  core_instance.HandleRequest(json);

  // {ID:0} отправляет запрос на продажу
  str = R"({"Message":{"Dollars":10,"Rubles":10,"UserId":0},"ReqType":"SELL"})";
  json = nlohmann::json::parse(str);
  core_instance.HandleRequest(json);

  // {ID:1} проверка баланса перед рассчётом
  str = R"({"Message":{"UserId":1},"ReqType":"BALANCE"})";
  json = nlohmann::json::parse(str);
  BOOST_REQUIRE_EQUAL(R"({"Code":"OK","Message":"Current balanse is 0 dollars and 0 rubles"})", core_instance.HandleRequest(json));
  
  // {ID:0} проверка баланса перед рассчётом
  str = R"({"Message":{"UserId":0},"ReqType":"BALANCE"})";
  json = nlohmann::json::parse(str);
  BOOST_REQUIRE_EQUAL(R"({"Code":"OK","Message":"Current balanse is 0 dollars and 0 rubles"})", core_instance.HandleRequest(json));

  // Вывод всех активных запросов перед рассчётом
  str = R"({"Message":{"UserId":0},"ReqType":"ACTIVE"})";
  json = nlohmann::json::parse(str);
  BOOST_REQUIRE_EQUAL(R"({"Code":"OK","Message":"All active transactions\nto SELL:\n\t id:2. 10 dollars for 10 rubles \nto BUY:\n\t id:1. 10 dollars for 10 rubles \n"})", core_instance.HandleRequest(json));

  // Проведение расчёта
  core_instance.TransactionManagment();

  // Вывод всех активных запросов после рассчёта
  str = R"({"Message":{"UserId":0},"ReqType":"ACTIVE"})";
  json = nlohmann::json::parse(str);
  BOOST_REQUIRE_EQUAL(R"({"Code":"OK","Message":"All active transactions\nto SELL:\nto BUY:\n"})", core_instance.HandleRequest(json));

  // {ID:1} проверка баланса после рассчёта
  str = R"({"Message":{"UserId":1},"ReqType":"BALANCE"})";
  json = nlohmann::json::parse(str);
  BOOST_REQUIRE_EQUAL(R"({"Code":"OK","Message":"Current balanse is 10 dollars and -100 rubles"})", core_instance.HandleRequest(json));
  
  // {ID:0} проверка баланса после рассчёта
  str = R"({"Message":{"UserId":0},"ReqType":"BALANCE"})";
  json = nlohmann::json::parse(str);
  BOOST_REQUIRE_EQUAL(R"({"Code":"OK","Message":"Current balanse is -10 dollars and 100 rubles"})", core_instance.HandleRequest(json));
}

// Тест-кейс частично закрытой сделки
BOOST_AUTO_TEST_CASE(HalfTransaction) {
  // Регистрируем пользователя {ID:0}
  std::string str = R"({"Message":{"Password":"23","UserId":0,"Username":"test"},"ReqType":"REG"})";
  nlohmann::json json = nlohmann::json::parse(str);
  core_instance.HandleRequest(json);

  // Регистрируем пользователя {ID:1}
  str = R"({"Message":{"Password":"32","UserId":0,"Username":"test1"},"ReqType":"REG"})";
  json = nlohmann::json::parse(str);
  core_instance.HandleRequest(json);

  // {ID:1} отправляет запрос на покупку
  str = R"({"Message":{"Dollars":6,"Rubles":10,"UserId":1},"ReqType":"BUY"})";
  json = nlohmann::json::parse(str);
  core_instance.HandleRequest(json);

  // {ID:0} отправляет запрос на продажу
  str = R"({"Message":{"Dollars":10,"Rubles":10,"UserId":0},"ReqType":"SELL"})";
  json = nlohmann::json::parse(str);
  core_instance.HandleRequest(json);

  // {ID:1} проверка баланса перед рассчётом
  str = R"({"Message":{"UserId":1},"ReqType":"BALANCE"})";
  json = nlohmann::json::parse(str);
  BOOST_REQUIRE_EQUAL(R"({"Code":"OK","Message":"Current balanse is 0 dollars and 0 rubles"})", core_instance.HandleRequest(json));
  
  // {ID:0} проверка баланса перед рассчётом
  str = R"({"Message":{"UserId":0},"ReqType":"BALANCE"})";
  json = nlohmann::json::parse(str);
  BOOST_REQUIRE_EQUAL(R"({"Code":"OK","Message":"Current balanse is 0 dollars and 0 rubles"})", core_instance.HandleRequest(json));

  // Вывод всех активных запросов перед рассчётом
  str = R"({"Message":{"UserId":0},"ReqType":"ACTIVE"})";
  json = nlohmann::json::parse(str);
  BOOST_REQUIRE_EQUAL(R"({"Code":"OK","Message":"All active transactions\nto SELL:\n\t id:2. 10 dollars for 10 rubles \nto BUY:\n\t id:1. 6 dollars for 10 rubles \n"})", core_instance.HandleRequest(json));

  // Проведение расчёта
  core_instance.TransactionManagment();

  // Вывод всех активных запросов после рассчёта
  str = R"({"Message":{"UserId":0},"ReqType":"ACTIVE"})";
  json = nlohmann::json::parse(str);
  BOOST_REQUIRE_EQUAL(R"({"Code":"OK","Message":"All active transactions\nto SELL:\n\t id:2. 4 dollars for 10 rubles \nto BUY:\n"})", core_instance.HandleRequest(json));

  // {ID:1} проверка баланса после рассчёта
  str = R"({"Message":{"UserId":1},"ReqType":"BALANCE"})";
  json = nlohmann::json::parse(str);
  BOOST_REQUIRE_EQUAL(R"({"Code":"OK","Message":"Current balanse is 6 dollars and -60 rubles"})", core_instance.HandleRequest(json));
  
  // {ID:0} проверка баланса после рассчёта
  str = R"({"Message":{"UserId":0},"ReqType":"BALANCE"})";
  json = nlohmann::json::parse(str);
  BOOST_REQUIRE_EQUAL(R"({"Code":"OK","Message":"Current balanse is -6 dollars and 60 rubles"})", core_instance.HandleRequest(json));
}

// Тест-кейс из примера
BOOST_AUTO_TEST_CASE(FromExmaple) {
  // Регистрируем пользователя {ID:0}
  std::string str = R"({"Message":{"Password":"23","UserId":0,"Username":"P1"},"ReqType":"REG"})";
  nlohmann::json json = nlohmann::json::parse(str);
  core_instance.HandleRequest(json);

  // Регистрируем пользователя {ID:1}
  str = R"({"Message":{"Password":"32","UserId":0,"Username":"P2"},"ReqType":"REG"})";
  json = nlohmann::json::parse(str);
  core_instance.HandleRequest(json);

  // Регистрируем пользователя {ID:2}
  str = R"({"Message":{"Password":"32","UserId":0,"Username":"P3"},"ReqType":"REG"})";
  json = nlohmann::json::parse(str);
  core_instance.HandleRequest(json);

  // {ID:0} отправляет запрос на продажу
  str = R"({"Message":{"Dollars":10,"Rubles":62,"UserId":0},"ReqType":"BUY"})";
  json = nlohmann::json::parse(str);
  core_instance.HandleRequest(json);

  // {ID:1} отправляет запрос на покупку
  str = R"({"Message":{"Dollars":20,"Rubles":63,"UserId":1},"ReqType":"BUY"})";
  json = nlohmann::json::parse(str);
  core_instance.HandleRequest(json);

  // {ID:2} отправляет запрос на продажу
  str = R"({"Message":{"Dollars":50,"Rubles":61,"UserId":2},"ReqType":"SELL"})";
  json = nlohmann::json::parse(str);
  core_instance.HandleRequest(json);

  // {ID:0} проверка баланса перед рассчётом
  str = R"({"Message":{"UserId":0},"ReqType":"BALANCE"})";
  json = nlohmann::json::parse(str);
  BOOST_REQUIRE_EQUAL(R"({"Code":"OK","Message":"Current balanse is 0 dollars and 0 rubles"})", core_instance.HandleRequest(json));

  // {ID:1} проверка баланса перед рассчётом
  str = R"({"Message":{"UserId":1},"ReqType":"BALANCE"})";
  json = nlohmann::json::parse(str);
  BOOST_REQUIRE_EQUAL(R"({"Code":"OK","Message":"Current balanse is 0 dollars and 0 rubles"})", core_instance.HandleRequest(json));

  // {ID:2} проверка баланса перед рассчётом
  str = R"({"Message":{"UserId":2},"ReqType":"BALANCE"})";
  json = nlohmann::json::parse(str);
  BOOST_REQUIRE_EQUAL(R"({"Code":"OK","Message":"Current balanse is 0 dollars and 0 rubles"})", core_instance.HandleRequest(json));

  // Проведение расчёта
  core_instance.TransactionManagment();

  // Вывод всех активных запросов после рассчёта
  str = R"({"Message":{"UserId":0},"ReqType":"ACTIVE"})";
  json = nlohmann::json::parse(str);
  BOOST_REQUIRE_EQUAL(R"({"Code":"OK","Message":"All active transactions\nto SELL:\n\t id:3. 20 dollars for 61 rubles \nto BUY:\n"})", core_instance.HandleRequest(json));

  // {ID:0} проверка баланса после рассчёта
  str = R"({"Message":{"UserId":0},"ReqType":"BALANCE"})";
  json = nlohmann::json::parse(str);
  BOOST_REQUIRE_EQUAL(R"({"Code":"OK","Message":"Current balanse is 10 dollars and -620 rubles"})", core_instance.HandleRequest(json));

  // {ID:1} проверка баланса после рассчёта
  str = R"({"Message":{"UserId":1},"ReqType":"BALANCE"})";
  json = nlohmann::json::parse(str);
  BOOST_REQUIRE_EQUAL(R"({"Code":"OK","Message":"Current balanse is 20 dollars and -1260 rubles"})", core_instance.HandleRequest(json));

  // {ID:2} проверка баланса после рассчёта
  str = R"({"Message":{"UserId":2},"ReqType":"BALANCE"})";
  json = nlohmann::json::parse(str);
  BOOST_REQUIRE_EQUAL(R"({"Code":"OK","Message":"Current balanse is -30 dollars and 1880 rubles"})", core_instance.HandleRequest(json));
}

BOOST_AUTO_TEST_SUITE_END()