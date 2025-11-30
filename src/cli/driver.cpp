/**
 * @file driver.cpp
 * @brief 编译驱动器实现。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-30
 */

#include "czc/cli/driver.hpp"
#include "czc/cli/output/formatter.hpp"
#include "czc/cli/phases/lexer_phase.hpp"

#include <fstream>
#include <iostream>

namespace czc::cli {

Driver::Driver() {
  // 设置默认诊断处理器
  ctx_.diagnostics().setHandler(
      [this](const Diagnostic &diag) { defaultDiagnosticPrinter(diag); });
}

Driver::Driver(CompilerContext ctx) : ctx_(std::move(ctx)) {
  // 设置默认诊断处理器
  ctx_.diagnostics().setHandler(
      [this](const Diagnostic &diag) { defaultDiagnosticPrinter(diag); });
}

void Driver::setDiagnosticPrinter(DiagnosticPrinter printer) {
  ctx_.diagnostics().setHandler(std::move(printer));
}

int Driver::runLexer(const std::filesystem::path &inputFile) {
  // 创建词法分析阶段
  LexerPhase phase(ctx_);

  // 执行词法分析
  auto result = phase.runOnFile(inputFile);

  if (!result.has_value()) {
    // 报告错误
    ctx_.diagnostics().error(result.error().message, result.error().code);
    return 1;
  }

  const auto &lexResult = result.value();

  // 格式化输出
  auto formatter = createFormatter(ctx_.output().format);
  std::string output;

  if (lexResult.hasErrors) {
    // 错误已通过诊断系统报告，这里只需返回错误码
    return 1;
  }

  // 格式化 Token 输出
  output = formatter->formatTokens(lexResult.tokens, phase.sourceManager());

  // 输出结果
  if (ctx_.output().file.has_value()) {
    std::ofstream ofs(ctx_.output().file.value());
    if (!ofs) {
      ctx_.diagnostics().error("Failed to open output file: " +
                                   ctx_.output().file.value().string(),
                               "E010");
      return 1;
    }
    ofs << output;
  } else {
    std::cout << output;
  }

  return 0;
}

void Driver::printDiagnosticSummary() const {
  const auto &diag = ctx_.diagnostics();

  if (diag.errorCount() > 0 || diag.warningCount() > 0) {
    *errStream_ << "\n";
    if (diag.errorCount() > 0) {
      *errStream_ << diag.errorCount() << " error(s)";
      if (diag.warningCount() > 0) {
        *errStream_ << ", ";
      }
    }
    if (diag.warningCount() > 0) {
      *errStream_ << diag.warningCount() << " warning(s)";
    }
    *errStream_ << " generated.\n";
  }
}

void Driver::defaultDiagnosticPrinter(const Diagnostic &diag) const {
  // 只有非静默模式才输出
  if (ctx_.isQuiet() && diag.level == DiagnosticLevel::Note) {
    return;
  }

  // 颜色输出（如果启用）
  const bool useColor = ctx_.global().colorDiagnostics;

  if (useColor) {
    switch (diag.level) {
    case DiagnosticLevel::Note:
      *errStream_ << "\033[36m"; // Cyan
      break;
    case DiagnosticLevel::Warning:
      *errStream_ << "\033[33m"; // Yellow
      break;
    case DiagnosticLevel::Error:
    case DiagnosticLevel::Fatal:
      *errStream_ << "\033[31m"; // Red
      break;
    }
  }

  *errStream_ << diag.format();

  if (useColor) {
    *errStream_ << "\033[0m"; // Reset
  }

  *errStream_ << "\n";
}

} // namespace czc::cli
