#include "Skeleton.h"
#include "Node.h"
#include"Animation.h"
#include "MT_Matrix.h"
#include"LineManager.h"

int32_t Skeleton::CreateJoint(const Node& node, const std::optional<int32_t>& parent, std::vector<Joint>& joints) {

	Joint joint;
	joint.name = node.name;
	joint.localMatrix = node.localMatrix;
	joint.skeletonSpaceMatrix = MakeIdentity4x4();
	joint.transform = node.transform;
	joint.index = static_cast<int32_t>(joints.size());
	joint.parentIndex = parent;
	joints.push_back(joint);
	for (const Node& child : node.children) {
		int32_t childIndex = CreateJoint(child, joint.index, joints);
		joint.childrenIndex.push_back(childIndex);
	}

	return joint.index;



}

Skeleton::SkeletonData Skeleton::CreateSkelton(const Node& rootNode) {

	SkeletonData skeleton;
	skeleton.rootIndex = CreateJoint(rootNode, {}, skeleton.joints);

	for (const Joint& joint : skeleton.joints) {
		skeleton.jointMap.emplace(joint.name, joint.index);
	}

	return skeleton;


}

void Skeleton::Update(SkeletonData& skeleton) {
	for (Joint& joint : skeleton.joints) {
		joint.localMatrix = MakeAffineMatrix(joint.transform.scale, joint.transform.rotate.ToEuler(), joint.transform.translate);
		if (joint.parentIndex) {
			joint.skeletonSpaceMatrix = skeleton.joints[*joint.parentIndex].skeletonSpaceMatrix * joint.localMatrix;
		} else {
			joint.skeletonSpaceMatrix = joint.localMatrix; // ルートジョイントは自身のローカル行列がスケルトンスペース行列になる
		}
	}

}


void Skeleton::DrawSkeletonLines(const Skeleton::SkeletonData& skeleton, const Vector4& color) {
	
	for (const auto& joint : skeleton.joints) {
		if (joint.parentIndex) {
			const auto& parent = skeleton.joints[*joint.parentIndex];
			// 親と自分のワールド座標を抽出
			Vector3 parentPos = {parent.skeletonSpaceMatrix.m[3][0], parent.skeletonSpaceMatrix.m[3][1], parent.skeletonSpaceMatrix.m[3][2]};
			Vector3 jointPos = {joint.skeletonSpaceMatrix.m[3][0], joint.skeletonSpaceMatrix.m[3][1], joint.skeletonSpaceMatrix.m[3][2]};
			LineManager::GetInstance()->DrawLine(parentPos, jointPos, color);
		}
	}
}