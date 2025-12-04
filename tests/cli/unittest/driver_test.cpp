/**
 * @file driver_test.cpp
 * @brief Driver 单元测试。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-12-04
 */

#include "czc/cli/driver.hpp"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <sstream>

namespace czc::cli {
namespace {

class DriverTest : public ::testing::Test {
protected:
  Driver driver_;
  std::filesystem::path testDir_;

  void SetUp() override {
    // 创建临时测试目录
    testDir_ = std::filesystem::temp_directory_path() / "czc_driver_test";
    std::filesystem::create_directories(testDir_);

    // 使用自定义的诊断处理器来捕获诊断信息
    diagnostics_.clear();
    driver_.setDiagnosticPrinter(
        [this](const Diagnostic &diag) { diagnostics_.push_back(diag); });
  }

  void TearDown() override {
    // 清理临时测试目录
    std::filesystem::remove_all(testDir_);
  }

  /**
   * @brief 创建临时测试文件。
   */
  std::filesystem::path createTestFile(std::string_view filename,
                                        std::string_view content) {
    auto path = testDir_ / filename;
    std::ofstream ofs(path);
    ofs << content;
    return path;
  }

  std::vector<Diagnostic> diagnostics_;
};

// ============================================================================
// 配置测试
// ============================================================================

TEST_F(DriverTest, DefaultConfiguration) {
  EXPECT_EQ(driver_.context().global().logLevel, LogLevel::Normal);
  EXPECT_EQ(driver_.context().output().format, OutputFormat::Text);
}

TEST_F(DriverTest, SetVerbose) {
  driver_.setVerbose(true);
  EXPECT_EQ(driver_.context().global().logLevel, LogLevel::Verbose);

  driver_.setVerbose(false);
  EXPECT_EQ(driver_.context().global().logLevel, LogLevel::Normal);
}

TEST_F(DriverTest, SetQuiet) {
  driver_.setQuiet(true);
  EXPECT_EQ(driver_.context().global().logLevel, LogLevel::Quiet);
}

TEST_F(DriverTest, SetOutputFormat) {
  driver_.setOutputFormat(OutputFormat::Json);
  EXPECT_EQ(driver_.context().output().format, OutputFormat::Json);
}

TEST_F(DriverTest, SetOutputFile) {
  std::filesystem::path path = "/tmp/test_output.txt";
  driver_.setOutputFile(path);
  EXPECT_EQ(driver_.context().output().file.value(), path);
}

TEST_F(DriverTest, SetColorDiagnostics) {
  driver_.setColorDiagnostics(false);
  EXPECT_FALSE(driver_.context().global().colorDiagnostics);

  driver_.setColorDiagnostics(true);
  EXPECT_TRUE(driver_.context().global().colorDiagnostics);
}

// ============================================================================
// runLexer 测试
// ============================================================================

TEST_F(DriverTest, RunLexerOnValidFile) {
  auto path = createTestFile("valid.zero", "let x = 1;");

  int exitCode = driver_.runLexer(path);

  EXPECT_EQ(exitCode, 0);
  EXPECT_TRUE(diagnostics_.empty());
}

TEST_F(DriverTest, RunLexerOnNonExistentFile) {
  std::filesystem::path nonExistent = testDir_ / "does_not_exist.zero";

  int exitCode = driver_.runLexer(nonExistent);

  EXPECT_NE(exitCode, 0);
  EXPECT_FALSE(diagnostics_.empty());
  EXPECT_EQ(diagnostics_[0].level, DiagnosticLevel::Error);
}

TEST_F(DriverTest, RunLexerWithErrors) {
  auto path = createTestFile("error.zero", R"(
let s = "unterminated string
)");

  int exitCode = driver_.runLexer(path);

  EXPECT_NE(exitCode, 0);
  // 应该有错误诊断
  bool hasError = false;
  for (const auto &diag : diagnostics_) {
    if (diag.level == DiagnosticLevel::Error) {
      hasError = true;
      break;
    }
  }
  EXPECT_TRUE(hasError);
}

TEST_F(DriverTest, RunLexerOutputToFile) {
  auto inputPath = createTestFile("input.zero", "let x = 1;");
  auto outputPath = testDir_ / "output.txt";

  driver_.setOutputFile(outputPath);
  int exitCode = driver_.runLexer(inputPath);

  EXPECT_EQ(exitCode, 0);
  EXPECT_TRUE(std::filesystem::exists(outputPath));

  // 验证输出文件不为空
  std::ifstream ifs(outputPath);
  std::string content((std::istreambuf_iterator<char>(ifs)),
                      std::istreambuf_iterator<char>());
  EXPECT_FALSE(content.empty());
}

// ============================================================================
// 诊断测试
// ============================================================================

TEST_F(DriverTest, DiagnosticHandler) {
  auto path = createTestFile("valid.zero", "let x = 1;");

  // 手动添加一个诊断
  driver_.diagnostics().warning("test warning", "W001");
  driver_.runLexer(path);

  bool hasWarning = false;
  for (const auto &diag : diagnostics_) {
    if (diag.level == DiagnosticLevel::Warning) {
      hasWarning = true;
      break;
    }
  }
  EXPECT_TRUE(hasWarning);
}

TEST_F(DriverTest, ErrorStreamConfiguration) {
  std::ostringstream oss;
  driver_.setErrorStream(oss);

  // 使用默认诊断处理器
  driver_.setDiagnosticPrinter(
      [&oss](const Diagnostic &diag) { oss << diag.format() << "\n"; });

  driver_.diagnostics().error("test error message", "E999");

  std::string output = oss.str();
  EXPECT_NE(output.find("test error message"), std::string::npos);
}

// ============================================================================
// 移动语义测试
// ============================================================================

TEST_F(DriverTest, MoveConstruct) {
  driver_.setVerbose(true);
  Driver moved(std::move(driver_));

  EXPECT_EQ(moved.context().global().logLevel, LogLevel::Verbose);
}

} // namespace
} // namespace czc::cli
