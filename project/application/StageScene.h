// ...existing code...
#include "application/UI/Guide/GuideUI.h"
+
+// Player ring UI
+#include "application/UI/PlayerStatusRingUI/PlayerStatusRingUI.h"

#include <string>
#include <vector>


class StageScene : public IScene {
// ...existing code...
private:
	// ...existing code...
	std::unique_ptr<HpUI> hpUI_;
	std::unique_ptr<EnemyHpUI> enemyHpUI_;
	std::unique_ptr<GuideUI> guideUI_;
+	std::unique_ptr<PlayerStatusRingUI> playerRingUI_;

	// Multi-stage layout data (debug / editor)
	bool useMultiStageLayout_ = true;
// ...existing code...
};
