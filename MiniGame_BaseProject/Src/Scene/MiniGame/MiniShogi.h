#pragma once
#include <memory>
#include "GameBase.h"

#include "../../Object/Actor/MiniShogi/Piece.h"

class Shogiban;
class Komadai;

class Cursor;
class Selector;
class MiniShogiBoard;
class MiniShogiRule;
class Hand;
class MiniShogiActor;
class MiniShogiCpu;

enum class PromotionState
{
    NONE,
    WAIT_SELECT
};

enum class GameOverReason
{
    NONE,
    CHECKMATE,
    NO_LEGAL_MOVE,
    KING_MISSING
};

class MiniShogi :
    public GameBase
{
public:

    MiniShogi(void);
    ~MiniShogi(void);

    void Init(void) override;
    void Update(void) override;
    void Draw(void) override;
    void DrawUI(void) override;
    void Reset(void) override;

private:


    std::unique_ptr<Shogiban> shogiban_;
    std::unique_ptr<Komadai> komadai_;

    std::unique_ptr<Cursor> cursor_;
    std::unique_ptr<Selector> selector_;
    std::unique_ptr<MiniShogiBoard> board_;
    std::unique_ptr<MiniShogiRule> rule_;

    std::unique_ptr<Hand> player0Hand_;
    std::unique_ptr<Hand> player1Hand_;

    std::unique_ptr<MiniShogiActor> actor_;

	std::unique_ptr<MiniShogiCpu> cpu_;

    bool isPlayerTurn_;

    PromotionState promotionState_;

    GameOverReason gameOverReason_;

    bool promoteSelect_;

    Piece pendingMovePiece_;

    int pendingFromX_;
    int pendingFromY_;
    int pendingToX_;
    int pendingToY_;

	int cpuWaitFrame_;

    const char* ruleMessage_;
    int ruleMessageFrame_;

    bool isGameOver_;

    int gameOverFrame_;

    int fontTitle_;
    int fontMain_;

    bool isPlayerWin_;

    void InputUpdate(void);
	void UpdateCameraState(void);

    void SelectUpdate(void);

    void UpdatePromotion(void);

    void SelectBoardPiece(void);
    void MoveBoardPiece(void);
    void SelectHandPiece(void);
    void DropHandPiece(void);

    bool MovePiece(int fromX, int fromY, int toX, int toY);

    Hand& GetCurrentHand(void);
    const Hand& GetCurrentHand(void) const;

	bool ExecuteMove(int fromX, int fromY, int toX, int toY);
	bool ExecuteDrop(PieceType pieceType, int toX, int toY);

	void CpuUpdate(void);

    bool ExecuteCpuMove(
        int fromX,
        int fromY,
        int toX,
        int toY,
        bool isPromote
    );

    void CheckGameOver(void);
    void UpdateGameOver(void);
    void CancelSelect(void);
};

