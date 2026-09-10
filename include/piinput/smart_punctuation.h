#pragma once

#include <string>
#include <string_view>

namespace piinput {

// Smart punctuation is deliberately independent of TSF and the Host.  The
// caller supplies a stable text snapshot; this engine supplies only a semantic
// decision.  That keeps application callback quirks out of the punctuation
// rules and makes every rule directly testable.
enum class SmartPunctuationAction {
    transform,
    literal,
    provisional,
};

struct SmartPunctuationContext final {
    char symbol{};
    std::string_view left_text;
    std::string_view right_text;
    bool composing{};
};

struct SmartPunctuationDecision final {
    SmartPunctuationAction action{SmartPunctuationAction::transform};
    std::string_view rule_id{"PUNC-DEFAULT"};
    std::string_view chinese_text;
    std::string_view context_type{"UNKNOWN"};
};

struct SmartPunctuationResolution final {
    bool keep_ascii{};
    std::string_view rule_id{"PUNC-PENDING-CHINESE"};
    std::string_view chinese_text;
    bool continue_provisional{};
};

class SmartPunctuationEngine final {
public:
    [[nodiscard]] SmartPunctuationDecision decide(
        const SmartPunctuationContext& context) const noexcept;
    [[nodiscard]] SmartPunctuationResolution resolve_provisional(
        char symbol,
        char next_character,
        std::string_view provisional_rule_id = {},
        std::string_view accumulated_text = {}) const noexcept;

    [[nodiscard]] static bool is_ascii_digit(char value) noexcept;

    // 两键规则的判据本身：这个符号紧跟在 `preceding` 之后时，第一次按下取不取
    // ASCII 形式。只有句号和冒号有这条规则，判的是前一个字节是不是 ASCII 字母
    // 或数字——中文字符的最后一个字节不是 ASCII，所以中文行文碰不到它。
    //
    // 抽成公开静态，是因为有两处需要问同一个问题而输入不同：Shim 问的是文档里
    // 光标前面是什么，Host 问的是这次按键会提交出去的是什么。合成串上屏时符号
    // 紧跟的是刚提交的那段文本，而那段文本只有 Host 知道——它可能是选中的候选
    // （你好），也可能是原样上屏的输入（geek）。规则只该有一份定义。
    [[nodiscard]] static bool first_key_is_ascii(
        std::string_view preceding,
        char symbol) noexcept;

private:
    // decide() trims the right context to this line before calling this, so
    // every rule inside reads `right_text` as "text the caret sits in front
    // of" rather than "anything at all after the caret".
    [[nodiscard]] SmartPunctuationDecision decide_on_line(
        const SmartPunctuationContext& context) const noexcept;
};

}  // namespace piinput
