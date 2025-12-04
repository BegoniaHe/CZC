/**
 * @file context_test.cpp
 * @brief CompilerContext 单元测试。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-12-04
 */

#include "czc/cli/context.hpp"

#include <gtest/gtest.h>

namespace czc::cli {
namespace {

class CompilerContextTest : public ::testing::Test {
protected:
  CompilerContext ctx_;
};

// ============================================================================
// GlobalOptions 测试
// ============================================================================

TEST_F(CompilerContextTest, DefaultGlobalOptions) {
  auto &global = ctx_.global();

  EXPECT_EQ(global.logLevel, LogLevel::Normal);
  EXPECT_TRUE(global.colorDiagnostics);
}

TEST_F(CompilerContextTest, ModifyGlobalOptions) {
  ctx_.global().logLevel = LogLevel::Verbose;
  ctx_.global().colorDiagnostics = false;

  EXPECT_EQ(ctx_.global().logLevel, LogLevel::Verbose);
  EXPECT_FALSE(ctx_.global().colorDiagnostics);
}

TEST_F(CompilerContextTest, IsVerbose) {
  EXPECT_FALSE(ctx_.isVerbose());

  ctx_.global().logLevel = LogLevel::Verbose;
  EXPECT_TRUE(ctx_.isVerbose());

  ctx_.global().logLevel = LogLevel::Debug;
  EXPECT_TRUE(ctx_.isVerbose());
}

TEST_F(CompilerContextTest, IsQuiet) {
  EXPECT_FALSE(ctx_.isQuiet());

  ctx_.global().logLevel = LogLevel::Quiet;
  EXPECT_TRUE(ctx_.isQuiet());
}

// ============================================================================
// OutputOptions 测试
// ============================================================================

TEST_F(CompilerContextTest, DefaultOutputOptions) {
  auto &output = ctx_.output();

  EXPECT_FALSE(output.file.has_value());
  EXPECT_EQ(output.format, OutputFormat::Text);
}

TEST_F(CompilerContextTest, SetOutputFile) {
  ctx_.output().file = std::filesystem::path("/tmp/output.txt");

  EXPECT_TRUE(ctx_.output().file.has_value());
  EXPECT_EQ(ctx_.output().file.value().string(), "/tmp/output.txt");
}

TEST_F(CompilerContextTest, SetOutputFormat) {
  ctx_.output().format = OutputFormat::Json;

  EXPECT_EQ(ctx_.output().format, OutputFormat::Json);
}

// ============================================================================
// LexerOptions 测试
// ============================================================================

TEST_F(CompilerContextTest, DefaultLexerOptions) {
  auto &lexer = ctx_.lexer();

  EXPECT_FALSE(lexer.preserveTrivia);
  EXPECT_FALSE(lexer.dumpTokens);
}

TEST_F(CompilerContextTest, ModifyLexerOptions) {
  ctx_.lexer().preserveTrivia = true;
  ctx_.lexer().dumpTokens = true;

  EXPECT_TRUE(ctx_.lexer().preserveTrivia);
  EXPECT_TRUE(ctx_.lexer().dumpTokens);
}

// ============================================================================
// DiagnosticsEngine 测试
// ============================================================================

TEST_F(CompilerContextTest, DiagnosticsInitialState) {
  EXPECT_EQ(ctx_.diagnostics().errorCount(), 0u);
  EXPECT_EQ(ctx_.diagnostics().warningCount(), 0u);
  EXPECT_FALSE(ctx_.diagnostics().hasErrors());
}

TEST_F(CompilerContextTest, ReportError) {
  ctx_.diagnostics().error("test error", "E001");

  EXPECT_EQ(ctx_.diagnostics().errorCount(), 1u);
  EXPECT_TRUE(ctx_.diagnostics().hasErrors());
}

TEST_F(CompilerContextTest, ReportWarning) {
  ctx_.diagnostics().warning("test warning", "W001");

  EXPECT_EQ(ctx_.diagnostics().warningCount(), 1u);
  EXPECT_FALSE(ctx_.diagnostics().hasErrors());
}

TEST_F(CompilerContextTest, ClearDiagnostics) {
  ctx_.diagnostics().error("test error", "E001");
  ctx_.diagnostics().warning("test warning", "W001");

  ctx_.diagnostics().clear();

  EXPECT_EQ(ctx_.diagnostics().errorCount(), 0u);
  EXPECT_EQ(ctx_.diagnostics().warningCount(), 0u);
}

} // namespace
} // namespace czc::cli
