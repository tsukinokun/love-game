//--------------------------------------------------------------
//! @file   Runner.cpp
//! @brief  台本の実行器の実装
//--------------------------------------------------------------
#include <Script/Runner.hpp>

namespace LoveGame::Script {
    namespace {
        //! @brief ジャンプの輪で固まらないようにする、1回の Next で進める命令数の上限
        constexpr int kMaxStepsPerNext = 100000;

        int Read(const Variables& variables, const std::string& name) {
            const auto it = variables.find(name);
            return it != variables.end() ? it->second : 0;
        }

        bool Evaluate(int left, CompareOp op, int right) {
            switch(op) {
                case CompareOp::Equal: return left == right;
                case CompareOp::NotEqual: return left != right;
                case CompareOp::Less: return left < right;
                case CompareOp::LessEqual: return left <= right;
                case CompareOp::Greater: return left > right;
                case CompareOp::GreaterEqual: return left >= right;
            }
            return false;
        }
    }    // namespace

    void Apply(const Assignment& assignment, Variables& variables) {
        int& value = variables[assignment.variable];
        switch(assignment.op) {
            case AssignOp::Set: value = assignment.value; break;
            case AssignOp::Add: value += assignment.value; break;
            case AssignOp::Subtract: value -= assignment.value; break;
        }
    }

    Runner::Runner(const Program& program, Variables& variables) : m_program(&program), m_variables(&variables) {}

    bool Runner::JumpTo(const std::string& label) {
        const auto it = m_program->labels.find(label);
        if(it == m_program->labels.end()) return false;
        m_pc = static_cast<std::size_t>(it->second);
        return true;
    }

    bool Runner::StartAt(const std::string& label) {
        m_pendingChoice = nullptr;
        m_currentLineId.clear();
        return JumpTo(label);
    }

    bool Runner::ResumeAt(const std::string& lineId) {
        const auto it = m_program->lineIds.find(lineId);
        if(it == m_program->lineIds.end()) return false;
        m_pendingChoice = nullptr;
        m_pc            = static_cast<std::size_t>(it->second);
        return true;
    }

    const Op* Runner::Next() {
        for(int step = 0; step < kMaxStepsPerNext; ++step) {
            if(m_pc >= m_program->ops.size()) return nullptr;

            const Op& op = m_program->ops[m_pc];
            switch(op.type) {
                case OpType::Jump:
                    if(!JumpTo(op.targetLabel)) return nullptr;
                    break;
                case OpType::If:
                    if(Evaluate(Read(*m_variables, op.conditionVariable), op.conditionOp, op.conditionValue)) {
                        if(!JumpTo(op.targetLabel)) return nullptr;
                    } else {
                        ++m_pc;
                    }
                    break;
                case OpType::Set:
                    Apply(op.assignment, *m_variables);
                    ++m_pc;
                    break;
                case OpType::Choice:
                    m_currentLineId = op.lineId;
                    m_pendingChoice = &op;
                    return &op;
                case OpType::Text:
                    m_currentLineId = op.lineId;
                    ++m_pc;
                    return &op;
                case OpType::Command:
                    ++m_pc;
                    return &op;
                case OpType::End:
                    return &op;
            }
        }
        return nullptr;
    }

    bool Runner::Choose(std::size_t optionIndex) {
        if(m_pendingChoice == nullptr || optionIndex >= m_pendingChoice->options.size()) return false;
        const ChoiceOption& option = m_pendingChoice->options[optionIndex];
        for(const Assignment& effect : option.effects) Apply(effect, *m_variables);
        m_pendingChoice = nullptr;
        return JumpTo(option.targetLabel);
    }
}    // namespace LoveGame::Script
