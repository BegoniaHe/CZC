/**
 * @file lex_command.cpp
 * @brief 词法分析命令实现。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-30
 */

#include "czc/cli/commands/lex_command.hpp"
#include "czc/cli/options.hpp"
#include "czc/cli/output/formatter.hpp"
#include "czc/lexer/lexer.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

namespace czc::cli {

void LexCommand::setup(CLI::App *app) {
  // 输入文件（位置参数）
  app->add_option("input", inputFile_, "Input source file")
      ->required()
      ->check(CLI::ExistingFile);

  // trivia 模式
  app->add_flag("--trivia,-t", trivia_, "Preserve whitespace and comments")
      ->group("Lexer Options");

  // dump tokens
  app->add_flag("--dump-tokens,-d", dumpTokens_, "Dump all tokens")
      ->group("Lexer Options");
}

Result<int> LexCommand::execute() {
  // 读取输入文件
  auto content_result = readInputFile();
  if (!content_result.has_value()) {
    return std::unexpected(content_result.error());
  }
  const auto &content = content_result.value();

  // 创建源码管理器和 Lexer
  lexer::SourceManager sm;
  auto buffer_id = sm.addBuffer(content, inputFile_.string());
  lexer::Lexer lex(sm, buffer_id);

  // 执行词法分析
  std::vector<lexer::Token> tokens;
  if (trivia_) {
    tokens = lex.tokenizeWithTrivia();
  } else {
    tokens = lex.tokenize();
  }

  // 获取选项
  const auto &opts = cliOptionsConst();

  // 创建格式化器
  auto formatter = createFormatter(opts.output.format);

  // 格式化输出
  std::string output;
  if (lex.hasErrors()) {
    output = formatter->formatErrors(lex.errors(), sm);
  } else {
    output = formatter->formatTokens(tokens, sm);
  }

  // 输出结果
  if (opts.output.file.has_value()) {
    std::ofstream ofs(opts.output.file.value());
    if (!ofs) {
      return err<int>("Failed to open output file: " +
                          opts.output.file.value().string(),
                      "E002");
    }
    ofs << output;
  } else {
    std::cout << output;
  }

  // 返回退出码
  return ok(lex.hasErrors() ? 1 : 0);
}

Result<std::any>
LexCommand::execute(std::any input, [[maybe_unused]] const PhaseOptions &opts) {
  // Pipeline 接口实现（预留）
  // 期望 input 为 std::string（源码内容）或 std::filesystem::path（文件路径）

  std::string content;

  if (auto *path = std::any_cast<std::filesystem::path>(&input)) {
    inputFile_ = *path;
    auto result = readInputFile();
    if (!result.has_value()) {
      return std::unexpected(result.error());
    }
    content = std::move(result.value());
  } else if (auto *src = std::any_cast<std::string>(&input)) {
    content = *src;
  } else {
    return err<std::any>("Invalid input type for LexCommand", "E003");
  }

  // 创建源码管理器和 Lexer
  lexer::SourceManager sm;
  auto buffer_id = sm.addBuffer(content, inputFile_.string());
  lexer::Lexer lex(sm, buffer_id);

  // 执行词法分析
  auto tokens = trivia_ ? lex.tokenizeWithTrivia() : lex.tokenize();

  if (lex.hasErrors()) {
    // 返回错误信息
    return err<std::any>("Lexical analysis failed", "E004");
  }

  // 返回 Token 列表（使用 std::any 包装）
  return ok<std::any>(std::move(tokens));
}

Result<std::string> LexCommand::readInputFile() const {
  std::ifstream ifs(inputFile_);
  if (!ifs) {
    return err<std::string>("Failed to open input file: " + inputFile_.string(),
                            "E001");
  }

  std::ostringstream oss;
  oss << ifs.rdbuf();
  return ok(oss.str());
}

} // namespace czc::cli
