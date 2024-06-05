#ifndef PERLIN_H
#define PERLIN_H

#include "rtweekend.h"

/// <summary>
/// 关于Perlin噪声生成，Perlin噪声用于生成逼真纹理和模拟自然现象的技术
/// 此代码通过生成随机浮点数和排列数组来创建Perlin噪声
/// </summary>

class perlin
{
public:
	perlin()
	{
		randfloat = new double[point_count]; // 运行时创建数组
		for (int i = 0; i < point_count; i++)
		{ // 初始化随机浮点数组
			randfloat[i] = random_double();
		}
		// 生成并存储三个排列数组，这些数组用于噪声计算中确定索引
		perm_x = perlin_generate_perm();
		perm_y = perlin_generate_perm();
		perm_z = perlin_generate_perm();
	}

	~perlin()
	{
		delete[] randfloat;
		delete[] perm_x;
		delete[] perm_y;
		delete[] perm_z;
	}

	// 给定一个三维点p，计算并返回该点的perlin噪声值
	double noise(const point3 &p) const
	{
		// 计算p在各坐标上的小数部分，即该点距离所在单元格边界的距离
		auto u = p.x() - floor(p.x());
		auto v = p.y() - floor(p.y());
		auto w = p.z() - floor(p.z());
		// 确定p点对应网格单元起始坐标
		auto i = int(floor(p.x()));
		auto j = int(floor(p.y()));
		auto k = int(floor(p.z()));

		double c[2][2][2];

		for (int di = 0; di < 2; di++)
		{
			for (int dj = 0; dj < 2; dj++)
			{
				for (int dk = 0; dk < 2; dk++)
				{
					c[di][dj][dk] = randfloat[perm_x[(i + di) & 255] ^ perm_y[(j + dj) & 255] ^ perm_z[(k + dk) & 255]];
				}
			}
		}

		return trilinear_interp(c, u, v, w);
	}

private:
	static const int point_count = 256; // 噪声的粒度
	double *randfloat;					// 用于存储随机浮点数
	// 三个指向整数数组的指针，用于存储三维空间的排列
	int *perm_x;
	int *perm_y;
	int *perm_z;

	static int *perlin_generate_perm()
	{ // 生成排列数组
		auto p = new int[point_count];

		for (int i = 0; i < point_count; i++)
			p[i] = i;

		permute(p, point_count);

		return p;
	}

	static void permute(int *p, int n)
	{ // 打乱数组的顺序
		for (int i = n - 1; i > 0; i--)
		{
			int target = random_int(0, i);
			int tmp = p[i];
			p[i] = p[target];
			p[target] = tmp;
		}
	}

	// 三线性插值
	static double trilinear_interp(double c[2][2][2], double u, double v, double w)
	{
		auto accm = 0.0;
		for (int i = 0; i < 2; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				for (int k = 0; k < 2; k++)
				{
					accm += (i * u + (1 - i) * (1 - u)) * (j * v + (1 - j) * (1 - v)) * (k * w + (1 - k) * (1 - w)) * c[i][j][k];
				}
			}
		}
		return accm;
	}
};

#endif
