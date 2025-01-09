#pragma once
#include "BaseRenderer.h"
#include <array>
#include <vector>
#include <list>
#include <string>
#include <unordered_map>

namespace Poorenderer{

	struct Edge;
	struct EdgesPair;
	struct Polygon;
	class ScanLineRenderer;

	struct Edge
	{
		float x;	// 边的上端点x坐标
		float z;    // 边上端点的深度
		int ymax;   // 边的上方的最近扫描线索引，即像素坐标
		int dy;		// 边跨越的扫描线数目
		float dx;   // -1/k

		Edge();
		// 约定 v0 为上端点，创建以v0与v1为顶点的边
		Edge(const glm::vec3& v0, const glm::vec3& v1, float dzx, float dzy, float height);
		Edge(const Edge& rhs);
	};
	struct Polygon {
		//	ax + by + cz + d = 0
		float a, b, c, d; // 多边形所在平面的方程系数
		std::array<glm::vec3, 3> vertices; // 三角形屏幕空间坐标
		size_t id; // 多边形的编号
		int dy; // 多边形跨越的剩余扫描线数目
		int ymax;// 多边形y轴最大坐标 
		uint8_t color[3]; // 多边形的颜色
		std::list<Edge> SortedEdgeTable; // 多边形的排序边表

		Polygon();
		Polygon(size_t primitiveIdx, const ScanLineRenderer& renderer);
		Polygon(const Polygon &rhs);

		// 获取当前扫描线与多边形相交的边对 EdgesPair
		EdgesPair GetIntersections();

	private:
		// 计算多边形的成员参数
		void CalculateParameters(size_t primitiveIdx, const ScanLineRenderer& renderer);
		// 创建多边形的排序边表:  排序方式ymax(边的上端点)(从大到小) dx(从小到大)
		void CreateSortedEdgeTable(size_t primitiveIdx, const ScanLineRenderer& renderer);

	};
	struct EdgesPair {
		// 与扫描线相交的多边形边对的信息
		float xl;	// 左交点的x坐标
		float dxl;  // 两相邻扫描线交点的x坐标之差
		int dyl;    // 以和左交点所在边相交的扫描线数为初值，以后向下每处理一条扫描线减 1 
		float xr, dxr; 
		int dyr;	// 右边的交点的三个对应分量，含义同上

		float zl;   // 左交点处多边形所在平面的深度值
		float dzx;  // 沿扫描线向右走过一个像素时，多边形所在平面的深度增量。对于平面方程，
					// dzx＝-a/c(c≠0)
		float dzy;  // 沿y方向向下移过一根扫描线时，多边形所在平面的深度增量。对于平面方程，
					//	dzy＝b/c(c≠0)；
		size_t id;  // 交点对所在的多边形的编号
	};

	class ScanLineRenderer : public BaseRenderer {
	public:
		friend class Polygon;
		void Rasterization() override;
	private:
		std::vector<std::list<Polygon>> PolygonsTable;		   // 多边形表 PolygonsTable
		std::unordered_map<size_t, Polygon> ActivePolygonTable;// 活化多边形表 <Polygon.id, Polygon>
		std::list<EdgesPair> ActiveEdgeTable;				   // 活化(相交)边(对)表
		// 创建多边形边表
		void CreatePolygonsTable();
	};
}