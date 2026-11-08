//--------------------------------------------------------------
//! @file   Validator.cpp
//! @brief  台本の意味の検証の実装
//--------------------------------------------------------------
#include <Script/Validator.hpp>

#include <charconv>
#include <unordered_map>

namespace LoveGame::Script {
    namespace {
        constexpr const char* kPlayerSpeaker = "{player}";

        //! @brief 演出命令ごとの引数の数
        const std::unordered_map<std::string, std::size_t>& CommandArity() {
            static const std::unordered_map<std::string, std::size_t> table = {
                {"stage", 1}, {"show", 2}, {"hide", 1}, {"camera", 1}, {"bgm", 1}, {"se", 1}, {"appoint", 2},
            };
            return table;
        }

        bool IsKnownSpeaker(const std::string& speaker, const CharacterTable& characters) {
            if(speaker == kPlayerSpeaker) return true;
            for(const CharacterDef& def : characters.characters) {
                if(def.name == speaker) return true;
            }
            return false;
        }
    }    // namespace

    bool ParsePositiveInt(const std::string& text, int& value) {
        const char* const end    = text.data() + text.size();
        const auto        result = std::from_chars(text.data(), end, value);
        return result.ec == std::errc{} && result.ptr == end && value >= 1;
    }

    std::vector<Diagnostic> Validate(const Program& program, const CharacterTable& characters, const StageConfig& stage) {
        std::vector<Diagnostic> errors;

        auto checkLabel = [&](int line, const std::string& label) {
            if(program.labels.find(label) == program.labels.end()) {
                errors.push_back({line, "飛び先のラベルがありません: " + label});
            }
        };

        for(const Op& op : program.ops) {
            switch(op.type) {
                case OpType::Text:
                    if(op.lineId.empty()) errors.push_back({op.sourceLine, "行IDがありません(--stamp で付けられます)"});
                    if(!op.speaker.empty() && !IsKnownSpeaker(op.speaker, characters)) {
                        errors.push_back({op.sourceLine, "未定義の話者です: " + op.speaker});
                    }
                    break;
                case OpType::Choice:
                    if(op.lineId.empty()) errors.push_back({op.sourceLine, "行IDがありません(--stamp で付けられます)"});
                    if(op.options.empty()) errors.push_back({op.sourceLine, "@choice に項目がありません"});
                    for(const ChoiceOption& option : op.options) checkLabel(option.sourceLine, option.targetLabel);
                    break;
                case OpType::Jump:
                case OpType::If:
                    checkLabel(op.sourceLine, op.targetLabel);
                    break;
                case OpType::Command: {
                    const auto it = CommandArity().find(op.command);
                    if(it == CommandArity().end()) {
                        errors.push_back({op.sourceLine, "未定義の命令です: @" + op.command});
                    } else if(op.args.size() != it->second) {
                        errors.push_back({op.sourceLine, "@" + op.command + " の引数の数が違います"});
                    } else if((op.command == "show" || op.command == "hide") && characters.FindById(op.args[0]) == nullptr) {
                        errors.push_back({op.sourceLine, "未定義のキャラです: " + op.args[0]});
                    } else if(op.command == "show" && stage.positions.count(op.args[1]) == 0) {
                        errors.push_back({op.sourceLine, "未定義の立ち位置です: " + op.args[1]});
                    } else if(op.command == "stage" && stage.stages.count(op.args[0]) == 0) {
                        errors.push_back({op.sourceLine, "未定義の舞台です: " + op.args[0]});
                    } else if(op.command == "camera" && stage.cameraShots.count(op.args[0]) == 0) {
                        errors.push_back({op.sourceLine, "未定義のカメラです: " + op.args[0]});
                    } else if(op.command == "appoint") {
                        checkLabel(op.sourceLine, op.args[0]);
                        int weeks = 0;
                        if(!ParsePositiveInt(op.args[1], weeks)) {
                            errors.push_back({op.sourceLine, "@appoint の週数は1以上の整数にしてください: " + op.args[1]});
                        }
                    }
                    break;
                }
                case OpType::Set:
                case OpType::End:
                    break;
            }
        }

        if(!program.ops.empty() && program.ops.back().type != OpType::End && program.ops.back().type != OpType::Jump) {
            errors.push_back({program.ops.back().sourceLine, "台本の末尾が @end または @jump ではありません"});
        }
        return errors;
    }
}    // namespace LoveGame::Script
