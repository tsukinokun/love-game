//--------------------------------------------------------------
//! @file   Runner.hpp
//! @brief  台本の実行器。描画を知らず、次に行うことをゲーム側へ渡すだけにする
//--------------------------------------------------------------
#pragma once
#include <Script/Program.hpp>

#include <cstddef>
#include <map>
#include <string>

namespace LoveGame::Script {
    //! @brief 台本の変数。未定義の変数は 0 として扱う
    using Variables = std::map<std::string, int>;

    //! @brief 変数へ代入する
    void Apply(const Assignment& assignment, Variables& variables);

    class Runner {
    public:
        //--------------------------------------------------------------
        //! @param  program   [in]     実行する台本。Runner より長く生存させること
        //! @param  variables [in,out] 読み書きする変数。Runner より長く生存させること
        //--------------------------------------------------------------
        Runner(const Program& program, Variables& variables);

        //! @brief ラベルから実行を始める。ラベルが無ければ false
        bool StartAt(const std::string& label);

        //! @brief 行IDの行から実行を再開する。行が消えていれば false
        bool ResumeAt(const std::string& lineId);

        //--------------------------------------------------------------
        //! @brief  次に止まる命令まで進める
        //! @return Text / Choice / Command / End のいずれか。
        //!         ポインタは program の中を指す。台本の末尾では nullptr
        //--------------------------------------------------------------
        const Op* Next();

        //! @brief 直前の Choice に答え、効果を適用して飛び先へ移る
        bool Choose(std::size_t optionIndex);

        //! @brief 最後に止まった台詞または選択肢の行ID（セーブ位置）
        const std::string& CurrentLineId() const { return m_currentLineId; }

    private:
        bool JumpTo(const std::string& label);

        const Program* m_program;
        Variables*     m_variables;
        std::size_t    m_pc            = 0;
        const Op*      m_pendingChoice = nullptr;
        std::string    m_currentLineId;
    };
}    // namespace LoveGame::Script
