#include <Account.h>
#include <Transaction.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <stdexcept>

using ::testing::_;
using ::testing::NiceMock;

class MockAccount : public Account {
 public:
  MockAccount(int id, int balance) : Account(id, balance) {}

  MOCK_METHOD0(Unlock, void());
};

class MockTransaction : public Transaction {
 public:
  MOCK_METHOD3(SaveToDataBase, void(Account& from, Account& to, int sum));
};

/*
 * Tests for Account
 */

TEST(AccountTest, ConstructorAndGetters) {
  Account account(10, 500);

  EXPECT_EQ(account.id(), 10);
  EXPECT_EQ(account.GetBalance(), 500);
}

TEST(AccountTest, ChangeBalanceWithoutLockThrows) {
  Account account(1, 100);

  EXPECT_THROW(account.ChangeBalance(50), std::runtime_error);
}

TEST(AccountTest, ChangeBalanceAfterLock) {
  Account account(1, 100);

  account.Lock();
  account.ChangeBalance(50);

  EXPECT_EQ(account.GetBalance(), 150);

  account.Unlock();
}

TEST(AccountTest, DoubleLockThrows) {
  Account account(1, 100);

  account.Lock();

  EXPECT_THROW(account.Lock(), std::runtime_error);

  account.Unlock();
}

TEST(AccountTest, UnlockAllowsLockAgain) {
  Account account(1, 100);

  account.Lock();
  account.Unlock();

  EXPECT_NO_THROW(account.Lock());

  account.Unlock();
}

TEST(AccountTest, MockObjectIsUsed) {
  NiceMock<MockAccount> account(1, 100);

  EXPECT_CALL(account, Unlock())
      .Times(1);

  account.Unlock();
}

/*
 * Tests for Transaction
 */

TEST(TransactionTest, DefaultFee) {
  Transaction transaction;

  EXPECT_EQ(transaction.fee(), 1);
}

TEST(TransactionTest, SetFee) {
  Transaction transaction;

  transaction.set_fee(10);

  EXPECT_EQ(transaction.fee(), 10);
}

TEST(TransactionTest, SameAccountThrows) {
  Account from(1, 500);
  Account to(1, 500);
  Transaction transaction;

  EXPECT_THROW(transaction.Make(from, to, 100), std::logic_error);
}

TEST(TransactionTest, NegativeSumThrows) {
  Account from(1, 500);
  Account to(2, 500);
  Transaction transaction;

  EXPECT_THROW(transaction.Make(from, to, -100), std::invalid_argument);
}

TEST(TransactionTest, TooSmallSumThrows) {
  Account from(1, 500);
  Account to(2, 500);
  Transaction transaction;

  EXPECT_THROW(transaction.Make(from, to, 50), std::logic_error);
}

TEST(TransactionTest, FeeTooLargeReturnsFalse) {
  Account from(1, 500);
  Account to(2, 500);
  Transaction transaction;

  transaction.set_fee(60);

  EXPECT_FALSE(transaction.Make(from, to, 100));
}

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

TEST(TransactionTest, FailedDebitRollsBackCredit) {
  Account from(1, 500);
  Account to(2, 0);
  MockTransaction transaction;

  EXPECT_CALL(transaction, SaveToDataBase(_, _, 100))
      .Times(1);

  EXPECT_FALSE(transaction.Make(from, to, 100));

  EXPECT_EQ(from.GetBalance(), 500);
  EXPECT_EQ(to.GetBalance(), 0);
}

TEST(TransactionTest, RealSaveToDataBase) {
  Account from(1, 500);
  Account to(2, 500);
  Transaction transaction;

  EXPECT_TRUE(transaction.Make(from, to, 100));
}
