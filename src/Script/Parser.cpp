//--------------------------------------------------------------
//! @file   Parser.cpp
//! @brief  台本テキストの解析と行IDの付与の実装
//--------------------------------------------------------------
#include <Script/Parser.hpp>

#include <charconv>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <unordered_set>

namespace LoveGame::Script {
    namespace {
        constexpr std::string_view kOpenQuote    = "「";
        constexpr std::string_view kCloseQuote   = "」";
        constexpr std::size_t      kLineIdLength = 4;

        std::string_view Trim(std::string_view s) {
            while(!s.empty() && (s.front() == ' ' || s.front() == '\t' || s.front() == '\r')) s.remove_prefix(1);
            while(!s.empty() && (s.back() == ' ' || s.back() == '\t' || s.back() == '\r')) s.remove_suffix(1);
            return s;
        }

        std::vector<std::string> SplitWords(std::string_view s) {
            std::vector<std::string> words;
            std::size_t              i = 0;
            while(i < s.size()) {
                while(i < s.size() && (s[i] == ' ' || s[i] == '\t')) ++i;
                const std::size_t start = i;
                while(i < s.size() && s[i] != ' ' && s[i] != '\t') ++i;
                if(i > start) words.emplace_back(s.substr(start, i - start));
            }
            return words;
        }

        bool ParseInt(std::string_view s, int& out) {
            if(!s.empty() && s.front() == '+') s.remove_prefix(1);
            if(s.empty()) return false;
            const char* end    = s.data() + s.size();
            const auto  result = std::from_chars(s.data(), end, out);
            return result.ec == std::errc() && result.ptr == end;
        }

        bool IsHexLower(char c) { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'); }

        //! @brief 行末の「 #xxxx」を切り離す。無ければ id は空のまま
        std::string_view SplitLineId(std::string_view line, std::string& id) {
            constexpr std::size_t kSuffix = kLineIdLength + 1;
            if(line.size() <= kSuffix) return line;
            const std::string_view tail = line.substr(line.size() - kSuffix);
            if(tail.front() != '#') return line;
            for(char c : tail.substr(1)) {
                if(!IsHexLower(c)) return line;
            }
            const std::string_view rest = line.substr(0, line.size() - kSuffix);
            if(rest.back() != ' ' && rest.back() != '\t') return line;
            id = std::string(tail.substr(1));
            return Trim(rest);
        }

        bool ParseAssignment(const std::vector<std::string>& words, Assignment& out) {
            if(words.size() != 3) return false;
            if(words[1] == "=") out.op = AssignOp::Set;
            else if(words[1] == "+=") out.op = AssignOp::Add;
            else if(words[1] == "-=") out.op = AssignOp::Subtract;
            else return false;
            out.variable = words[0];
            return ParseInt(words[2], out.value);
        }

        bool ParseCompare(const std::string& s, CompareOp& out) {
            if(s == "==") out = CompareOp::Equal;
            else if(s == "!=") out = CompareOp::NotEqual;
            else if(s == "<") out = CompareOp::Less;
            else if(s == "<=") out = CompareOp::LessEqual;
            else if(s == ">") out = CompareOp::Greater;
            else if(s == ">=") out = CompareOp::GreaterEqual;
            else return false;
            return true;
        }

        //! @brief 1行ずつ取り出す。行末の \r は残るので呼び出し側で扱う
        template <class Fn>
        void ForEachLine(std::string_view text, Fn&& fn) {
            std::size_t start = 0;
            int         index = 1;
            while(true) {
                std::size_t end = text.find('\n', start);
                if(end == std::string_view::npos) end = text.size();
                fn(index, text.substr(start, end - start));
                if(end == text.size()) break;
                start = end + 1;
                ++index;
            }
        }

        bool IsCommentOrBlank(std::string_view trimmed) { return trimmed.empty() || trimmed.front() == ';'; }

        //! @brief 行IDを持つ種類の行か（台詞、地の文、@choice）
        //! @param trimmed [in] 前後の空白を除いた行
        //! @param raw     [in] 元の行。字下げされた行は選択肢の効果なので対象外
        bool TakesLineId(std::string_view trimmed, std::string_view raw) {
            if(IsCommentOrBlank(trimmed)) return false;
            if(raw.front() == ' ' || raw.front() == '\t') return false;
            const char head = trimmed.front();
            if(head == '*' || head == '-') return false;
            if(head == '@') {
                const std::vector<std::string> words = SplitWords(trimmed);
                return !words.empty() && words[0] == "@choice";
            }
            return true;
        }

        void ParseTextLine(std::string_view body, Op& op) {
            op.type                = OpType::Text;
            const std::size_t open = body.find(kOpenQuote);
            const bool        hasClose =
                body.size() >= kCloseQuote.size() && body.substr(body.size() - kCloseQuote.size()) == kCloseQuote;
            if(open != std::string_view::npos && open > 0 && hasClose) {
                op.speaker                  = std::string(Trim(body.substr(0, open)));
                const std::size_t textStart = open + kOpenQuote.size();
                op.text = std::string(body.substr(textStart, body.size() - kCloseQuote.size() - textStart));
            } else {
                op.text = std::string(body);
            }
        }
    }    // namespace

    ParseResult Parse(const std::string& name, std::string_view text) {
        ParseResult result;
        Program&    program = result.program;
        program.name        = name;

        auto addError = [&result](int line, std::string message) { result.errors.push_back({line, std::move(message)}); };

        auto registerLineId = [&](int line, const std::string& id) {
            if(id.empty()) return;
            if(!program.lineIds.emplace(id, static_cast<int>(program.ops.size())).second) {
                addError(line, "行IDが重複しています: #" + id);
            }
        };

        // 直前の選択肢へ項目を足している間だけ true
        bool inChoice = false;

        ForEachLine(text, [&](int lineNumber, std::string_view raw) {
            const std::string_view line = Trim(raw);
            if(IsCommentOrBlank(line)) return;

            const bool indented = raw.front() == ' ' || raw.front() == '\t';

            // 選択肢の項目
            if(line.front() == '-') {
                if(!inChoice) {
                    addError(lineNumber, "@choice の外に選択肢の項目があります");
                    return;
                }
                const std::size_t arrow = line.rfind("->");
                if(arrow == std::string_view::npos) {
                    addError(lineNumber, "選択肢に飛び先(-> ラベル)がありません");
                    return;
                }
                ChoiceOption option;
                option.text        = std::string(Trim(line.substr(1, arrow - 1)));
                option.targetLabel = std::string(Trim(line.substr(arrow + 2)));
                option.sourceLine  = lineNumber;
                if(option.text.empty()) addError(lineNumber, "選択肢の文章が空です");
                program.ops.back().options.push_back(std::move(option));
                return;
            }

            // 選択肢の効果
            if(inChoice && indented) {
                Assignment assignment;
                if(program.ops.back().options.empty()) {
                    addError(lineNumber, "選択肢の項目より前に効果が書かれています");
                } else if(ParseAssignment(SplitWords(line), assignment)) {
                    program.ops.back().options.back().effects.push_back(std::move(assignment));
                } else {
                    addError(lineNumber, "選択肢の効果を解釈できません(例: affection.akari += 5)");
                }
                return;
            }
            inChoice = false;

            // ラベル
            if(line.front() == '*') {
                const std::string label(Trim(line.substr(1)));
                if(label.empty()) {
                    addError(lineNumber, "ラベル名が空です");
                } else if(!program.labels.emplace(label, static_cast<int>(program.ops.size())).second) {
                    addError(lineNumber, "ラベルが重複しています: " + label);
                }
                return;
            }

            Op op;
            op.sourceLine = lineNumber;

            std::string            lineId;
            const std::string_view body = TakesLineId(line, raw) ? SplitLineId(line, lineId) : line;
            op.lineId                   = lineId;

            // 命令
            if(body.front() == '@') {
                std::vector<std::string> words = SplitWords(body.substr(1));
                if(words.empty()) {
                    addError(lineNumber, "命令名がありません");
                    return;
                }
                const std::string command = words[0];
                words.erase(words.begin());

                if(command == "choice") {
                    op.type = OpType::Choice;
                    registerLineId(lineNumber, lineId);
                    program.ops.push_back(std::move(op));
                    inChoice = true;
                    return;
                }
                if(command == "jump") {
                    op.type = OpType::Jump;
                    if(words.size() == 1) op.targetLabel = words[0];
                    else addError(lineNumber, "@jump の書式: @jump ラベル");
                } else if(command == "end") {
                    op.type = OpType::End;
                } else if(command == "set") {
                    op.type = OpType::Set;
                    if(!ParseAssignment(words, op.assignment)) addError(lineNumber, "@set の書式: @set 変数 += 数値");
                } else if(command == "if") {
                    // @if 変数 比較 数値 -> ラベル
                    op.type = OpType::If;
                    if(words.size() != 5 || words[3] != "->" || !ParseCompare(words[1], op.conditionOp) ||
                       !ParseInt(words[2], op.conditionValue)) {
                        addError(lineNumber, "@if の書式: @if 変数 >= 数値 -> ラベル");
                    } else {
                        op.conditionVariable = words[0];
                        op.targetLabel       = words[4];
                    }
                } else {
                    op.type    = OpType::Command;
                    op.command = command;
                    op.args    = std::move(words);
                }
                program.ops.push_back(std::move(op));
                return;
            }

            // 台詞または地の文
            ParseTextLine(body, op);
            registerLineId(lineNumber, lineId);
            program.ops.push_back(std::move(op));
        });

        return result;
    }

    std::string StampLineIds(std::string_view text, int& added) {
        added = 0;

        // 既存の行IDを先に集め、新しいIDとの衝突を避ける
        std::unordered_set<std::string> used;
        ForEachLine(text, [&](int, std::string_view raw) {
            const std::string_view line = Trim(raw);
            if(!TakesLineId(line, raw)) return;
            std::string id;
            SplitLineId(line, id);
            if(!id.empty()) used.insert(id);
        });

        std::string out;
        out.reserve(text.size() + 256);

        ForEachLine(text, [&](int lineNumber, std::string_view raw) {
            if(lineNumber > 1) out.push_back('\n');

            const std::string_view line  = Trim(raw);
            const bool             takes = TakesLineId(line, raw);
            std::string            id;
            if(takes) SplitLineId(line, id);
            if(!takes || !id.empty()) {
                out.append(raw);
                return;
            }

            // 内容から決定的に作るので、同じ台本へ何度かけても結果が揺れない
            std::uint32_t hash = 2166136261u;
            for(unsigned char c : line) hash = (hash ^ c) * 16777619u;

            char buffer[8];
            do {
                std::snprintf(buffer, sizeof(buffer), "%04x", static_cast<unsigned>(hash & 0xffffu));
                ++hash;
            } while(!used.insert(buffer).second);

            const bool       hasCr   = raw.back() == '\r';
            std::string_view content = raw;
            if(hasCr) content.remove_suffix(1);
            out.append(content);
            out.append(" #");
            out.append(buffer);
            if(hasCr) out.push_back('\r');
            ++added;
        });

        return out;
    }

    bool ReadTextFile(const std::filesystem::path& path, std::string& out) {
        std::ifstream file(path, std::ios::binary);
        if(!file) return false;
        out.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
        if(out.size() >= 3 && out.compare(0, 3, "\xEF\xBB\xBF") == 0) out.erase(0, 3);
        return true;
    }
}    // namespace LoveGame::Script
