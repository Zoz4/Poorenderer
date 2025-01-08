#include "ScanLineRenderer.h"
#include <algorithm>

#include <stb_image_write.h>

namespace Poorenderer {

	void ScanLineRenderer::Rasterization()
	{
		LOGI("ScanLineRenderer Rasterization..");
		CreatePolygonsTable();

		// 对每条扫描线
		for (int y = viewport.height-1; y >= 0; --y)
		{	
			LOGD("line: {} ", y);
			for (auto& polygon : PolygonsTable[y]) {
				// 如果至少有一条扫描线经过该多边形
				if (polygon.dy > 0) {
					// 将其加入活化多边形表
					ActivePolygonTable[polygon.id] = polygon;
					// 获取 当前扫描线 与 该多边形 的相交边对EdgesPair, 加入活化边表
					// 此后，当前多边形的排序边表中，会去除两条边(相交的边对)，剩余一条边
					ActiveEdgeTable.push_back(ActivePolygonTable[polygon.id].GetIntersections());
				}
			}

			// 对活化边表中的每个边对
			for (auto iter = ActiveEdgeTable.begin(); iter != ActiveEdgeTable.end(); ++iter) {

				EdgesPair& ep = *iter;
				auto& polygon = ActivePolygonTable[ep.id];// 当前边对所对应的多边形

				// 如果 初始边对中 存在某条边 经过的扫描线数为 0
				if (ep.dyl <= 0 || ep.dyr <= 0) {
					if (ep.dyl <= 0 && ep.dyr > 0) { // 如果左侧的边没有经过扫描线
						if (polygon.SortedEdgeTable.size() == 1) { // 更新活化边对左侧的的信息为多边形排序边表的下一条边
							Edge e0 = polygon.SortedEdgeTable.front();
							polygon.SortedEdgeTable.pop_front(); // 从多边形的排序边表中移除该边
							ep.xl = e0.x;	// 更新信息
							ep.dxl = e0.dx;
							ep.dyl = e0.dy;
							ep.zl = e0.z;
						}
						else {
							//LOGI("Something Wrong");
						}
					}
					else if (ep.dyr <= 0 && ep.dyl > 0) { // 如右侧的边没有经过扫描线，则更新边对的右侧信息
						if (polygon.SortedEdgeTable.size() == 1) {
							Edge e1 = polygon.SortedEdgeTable.front();
							polygon.SortedEdgeTable.pop_front();
							ep.xr = e1.x;
							ep.dxr = e1.dx;
							ep.dyr = e1.dy;
						}
						else {
							//LOGI("Something Wrong");
						}
					}
					else { // 如果某一个多边形的与扫描线相交的初始边对 经过的扫描线数都是 0 
						// 可以推断出该多边形为相邻两条扫描线间 非常狭窄的多边形，
						// 当前一条以水平像素中心相连的扫描线无法采样到该多边形
						// 或许一个像素可以对应多条扫描线？
						// 当前版本，该边对下面更新活化边表的过程中丢弃
						LOGE("Sampling Frequency is not enough");
					}
				}

				// 通过当前相交边对更新 colorAttachment 与 depthBuffer
				float zx = ep.zl;
				for (int x = ep.xl; x <= ep.xr; ++x) {
					size_t idx = y * viewport.width + x;
					if (zx < depthBuffer[idx]) {
						depthBuffer[idx] = zx;
						colorAttachment[idx * 3]	 = polygon.color[0];
						colorAttachment[idx * 3 + 1] = polygon.color[1];
						colorAttachment[idx * 3 + 2] = polygon.color[2];
					}
					zx += ep.dzx;
				}

				// 当前边对处理完了一行扫描线，更新边对信息
				ep.dyl -= 1;
				ep.dyr -= 1;
				ep.xl += ep.dxl;
				ep.xr += ep.dxr;
				ep.zl += ep.dzx * ep.dxl + ep.dzy; // 更新zl
				
				// 当前行更新完成后，如果 相交边对中存在边 不在经过扫描线
				// 从多边形的活化边表中取出剩余的边，与相交边对中另一条经过扫描线的边组成新的相交边对
				if (ep.dyl <= 0 || ep.dyr <= 0)
				{
					if (ep.dyl <= 0 && ep.dyr > 0) {
						if (polygon.SortedEdgeTable.size() == 1) {
							Edge e0 = polygon.SortedEdgeTable.front();
							polygon.SortedEdgeTable.pop_front();
							ep.xl = e0.x;
							ep.dxl = e0.dx;
							ep.dyl = e0.dy;
							ep.zl = e0.z;
						}
						else {
							//LOGI("Something Wrong");
						}
					}
					else if (ep.dyr <= 0 && ep.dyl > 0) {
						if (polygon.SortedEdgeTable.size() == 1) {
							Edge e1 = polygon.SortedEdgeTable.front();
							polygon.SortedEdgeTable.pop_front();
							ep.xr = e1.x;
							ep.dxr = e1.dx;
							ep.dyr = e1.dy;
						}
						else {
							//LOGI("polygon.SortedEdgeTable.size(): {}", polygon.SortedEdgeTable.size());
							//LOGI("Something Wrong");
						}
					}
					else { // ep.dyl <= 0 && ep.dyr <= 0
						// 如果更新完成后，该相交边对全部不再经过扫描线
						// 将其从活化边表中移除
						iter = ActiveEdgeTable.erase(iter);
						--iter;
						// 并更新对应的活化多边形表
						polygon.dy -= 1;
						if (polygon.dy < 0) {
							ActivePolygonTable.erase(polygon.id);
						}
						continue; // 处理下一个相交边对
					}
				}

				// 当前行更新完成后，如果 该相交边对仍然有效，更新该 边对 对应的活化多边形
				polygon.dy -= 1;
				if (polygon.dy < 0) {
					ActivePolygonTable.erase(polygon.id);
					iter = ActiveEdgeTable.erase(iter);
					--iter;
				}

			}// 遍历活化边表结束

		}// 遍历扫描线结束

		stbi_flip_vertically_on_write(true);
		stbi_write_png((RESOURCES_DIR+FileName).c_str(), viewport.width, viewport.height, 3, colorAttachment.data(), viewport.width * 3);
	}
	void ScanLineRenderer::CreatePolygonsTable()
	{
		PolygonsTable.resize(static_cast<size_t>(viewport.height));

		for (size_t i = 0; i < primitives.size(); ++i) {
			if (primitives[i].discard)
				continue;
			Polygon polygon(i, *this);
			PolygonsTable[polygon.ymax].push_back(polygon);
		}
	}

    Polygon::Polygon():
        a(0), b(0), c(0), d(0),
		vertices(), id(0), dy(0), ymax(0), SortedEdgeTable() {
		color[0] = 0;
		color[1] = 0;
		color[2] = 0;
	}
	Polygon::Polygon(size_t primitiveIdx, const ScanLineRenderer& renderer):a(0), b(0), c(0), d(0), id(primitiveIdx), dy(0), ymax(0)
	{
		vertices[0] = renderer.screenPositions[renderer.primitives[primitiveIdx].indices[0]];
		vertices[1] = renderer.screenPositions[renderer.primitives[primitiveIdx].indices[1]];
		vertices[2] = renderer.screenPositions[renderer.primitives[primitiveIdx].indices[2]];

		CalculateParameters(primitiveIdx, renderer);
	}
	Polygon::Polygon(const Polygon& rhs)
	{
		vertices = rhs.vertices;
		id = rhs.id;
		a = rhs.a; b = rhs.b; c = rhs.c; d = rhs.d;
		dy = rhs.dy;
		ymax = rhs.ymax;
		color[0] = rhs.color[0];
		color[1] = rhs.color[1];
		color[2] = rhs.color[2];
		SortedEdgeTable = rhs.SortedEdgeTable;
	}
	EdgesPair Polygon::GetIntersections( )
	{
		EdgesPair edgesPair{};
		if (SortedEdgeTable.size() >= 2)
		{
			Edge e0 = SortedEdgeTable.front(); SortedEdgeTable.pop_front();
			Edge e1 = SortedEdgeTable.front(); SortedEdgeTable.pop_front();
			edgesPair.xl = e0.x;
			edgesPair.dxl = e0.dx;
			edgesPair.dyl = e0.dy;
			edgesPair.xr = e1.x;
			edgesPair.dxr = e1.dx;
			edgesPair.dyr = e1.dy;

			edgesPair.zl = e0.z;
			edgesPair.dzx = -a / c;
			edgesPair.dzy = b / c;
			edgesPair.id = id;
		}

		return edgesPair;
	}
	void Polygon::CalculateParameters(size_t primitiveIdx, const ScanLineRenderer& renderer)
	{
		glm::vec3 v1 = vertices[1] - vertices[0];
		glm::vec3 v2 = vertices[2] - vertices[0];
		
		// Color
		glm::vec3 worldNormal = glm::normalize(renderer.worldNormals[renderer.primitives[primitiveIdx].indices[0]]);
		glm::vec3 light = glm::normalize(glm::vec3(1, 1, 1));
		float intensity = std::max(0.f, glm::dot(light, worldNormal));
		color[0] = 255 * intensity;
		color[1] = 255 * intensity;
		color[2] = 255 * intensity;

		// Plane Equation
		a = v1.y * v2.z - v1.z * v2.y;
		b = v1.z * v2.x - v1.x * v2.z;
		c = v1.x * v2.y - v1.y * v2.x;
		d = - glm::dot(glm::vec3(a, b, c), vertices[0]);

		float minY = std::min(std::min(vertices[0].y, vertices[1].y), vertices[2].y);
		float maxY = std::max(std::max(vertices[0].y, vertices[1].y), vertices[2].y);
		// ymax: 多边形经过的最上方的扫描线坐标，即像素行索引
		ymax = std::clamp(int(maxY - 0.5f), 0, int(renderer.viewport.height - 1));
		// ymin: 多边形经过的最下方的扫描线坐标
		int ymin = std::clamp(int(minY +  0.5f), 0, int(renderer.viewport.height - 1));
		dy = ymax - ymin + 1;

		CreateSortedEdgeTable(primitiveIdx, renderer);
		
	}
	void Polygon::CreateSortedEdgeTable(size_t primitiveIdx, const ScanLineRenderer& renderer)
	{
		std::array<glm::vec3, 3>  v = vertices;
		// 先按顶点y值从大到小排序
		std::sort(v.begin(), v.end(), 
			[](const glm::vec3& lhs, const glm::vec3& rhs) {
				return rhs.y < lhs.y;
			}
		);
		float k1 = (v[0].y-v[1].y)/(v[0].x-v[1].x);
		float k2 = (v[0].y - v[2].y) / (v[0].x - v[2].x);
		float dx1 = -1.f / k1, dx2 = -1.f / k2;
		if (dx1 < dx2)
		{
			/*
				v0

			v1

				  v2
			*/
			SortedEdgeTable.push_back(Edge(v[0], v[1], -a / c, b / c, renderer.viewport.height));
			SortedEdgeTable.push_back(Edge(v[0], v[2], -a / c, b / c, renderer.viewport.height));
			SortedEdgeTable.push_back(Edge(v[1], v[2], -a / c, b / c, renderer.viewport.height));

		}
		else
		{
			/*
				v0

					v1

			 v2
			*/
			SortedEdgeTable.push_back(Edge(v[0], v[2], -a / c, b / c, renderer.viewport.height));
			SortedEdgeTable.push_back(Edge(v[0], v[1], -a / c, b / c, renderer.viewport.height));
			SortedEdgeTable.push_back(Edge(v[1], v[2], -a / c, b / c, renderer.viewport.height));
			
		}
	}

	Edge::Edge():x(0),z(0),ymax(0),dy(0), dx(0){}
	Edge::Edge(const glm::vec3& v0, const glm::vec3& v1, float dzx, float dzy, float height)
	{
		float minY = v1.y;
		float maxY = v0.y;

		float k = (v1.y - v0.y) / (v1.x - v0.x);
		dx = -1.f / k;

		ymax = std::clamp(int(maxY-0.5f), 0, int(height));
		int ymin = std::clamp(int(minY + 0.5f), 0, int(height));
		dy = ymax - ymin + 1;

		// y 的减少量
		float deltaY = v0.y - ((float)ymax + 0.5f);
		// x 的变化量
		float deltaX = dx * deltaY;

		x = v0.x + deltaX;
		z = v0.z + dzx * deltaX + dzy * deltaY;

	}
	Edge::Edge(const Edge& rhs)
	{
		x = rhs.x;
		dx = rhs.dx;
		ymax = rhs.ymax;
		dy = rhs.dy;
		z = rhs.z;
	}
}