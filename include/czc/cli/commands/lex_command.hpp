/**
 * @file lex_command.hpp
 * @brief 词法分析命令定义。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-30
 *
 * @details
 *   实现 `czc lex` 子命令，对源文件进行词法分析。
 */

#ifndef CZC_CLI_COMMANDS_LEX_COMMAND_HPP
#define CZC_CLI_COMMANDS_LEX_COMMAND_HPP

#if __cplusplus < 202302L
#error "C++23 or higher is required"
#endif

#include "czc/cli/commands/command.hpp"
#include "czc/cli/commands/compiler_phase.hpp"

#include <filesystem>
#include <string>

namespace czc::cli {

/**
 * @brief 词法分析命令。
 *
 * @details
 *   实现 `czc lex` 子命令，支持：
 *   - 基础词法分析
 *   - Trivia 模式（保留空白和注释）
 *   - 多种输出格式（Text/JSON）
 *
 *   同时实现 CompilerPhase 接口，为 Pipeline 预留扩展。
 */
class LexCommand : public Command, public CompilerPhase {
public:
  LexCommand() = default;
  ~LexCommand() override = default;

  // ========== Command 接口 ==========

  /**
   * @brief 设置命令行选项。
   *
   * @param app CLI11 子命令 App 指针
   */
  void setup(CLI::App *app) override;

  /**
   * @brief 执行词法分析命令。
   *
   * @return 退出码（0 成功，非 0 失败）
   */
  [[nodiscard]] Result<int> execute() override;

  /**
   * @brief 获取命令名称。
   *
   * @return "lex"
   */
  [[nodiscard]] std::string_view name() const noexcept override {
    return "lex";
  }

  /**
   * @brief 获取命令描述。
   *
   * @return 命令描述
   */
  [[nodiscard]] std::string_view description() const noexcept override {
    return "Perform lexical analysis on source file";
  }

  /**
   * @brief 获取关联的编译阶段。
   *
   * @return this 指针
   */
  [[nodiscard]] CompilerPhase *asPhase() noexcept override { return this; }

  /**
   * @brief 获取关联的编译阶段（常量版本）。
   *
   * @return this 指针
   */
  [[nodiscard]] const CompilerPhase *asPhase() const noexcept override {
    return this;
  }

  // ========== CompilerPhase 接口 ==========

  /**
   * @brief 获取输入数据类型。
   *
   * @return "source"
   */
  [[nodiscard]] std::string_view inputType() const noexcept override {
    return "source";
  }

  /**
   * @brief 获取输出数据类型。
   *
   * @return "tokens"
   */
  [[nodiscard]] std::string_view outputType() const noexcept override {
    return "tokens";
  }

  /**
   * @brief 执行词法分析阶段（Pipeline 接口）。
   *
   * @param input 输入数据（预期为源文件路径或源码内容）
   * @param opts 阶段选项
   * @return Token 列表，失败时返回错误
   */
  [[nodiscard]] Result<std::any> execute(std::any input,
                                         const PhaseOptions &opts) override;

private:
  std::filesystem::path inputFile_; ///< 输入文件路径
  bool trivia_{false};              ///< 是否保留 trivia
  bool dumpTokens_{false};          ///< 是否输出所有 token

  /**
   * @brief 读取输入文件内容。
   *
   * @return 文件内容，失败时返回错误
   */
  [[nodiscard]] Result<std::string> readInputFile() const;
};

} // namespace czc::cli

#endif // CZC_CLI_COMMANDS_LEX_COMMAND_HPP
