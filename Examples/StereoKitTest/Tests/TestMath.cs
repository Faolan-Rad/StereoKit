﻿using StereoKit;
using System;

class TestMath : ITest
{
	const float tolerance = 0.0001f;
	bool TestMatrixDecompose()
	{
		Vec3 translate = new Vec3(1, 2, 3);
		Quat rotate    = Quat.FromAngles(0, 90, 0);
		Vec3 scale     = new Vec3(1,2,1);
		Matrix mat = Matrix.TRS(translate, rotate, scale);

		Vec3 exTranslate = mat.Translation;
		Quat exRotate    = mat.Rotation;
		Vec3 exScale     = mat.Scale;

		Log.Info($"Transform matrix:     {translate  } {rotate  } {scale  }");
		Log.Info($"Matrix sep decompose: {exTranslate} {exRotate} {exScale}");

		if (Vec3.Distance(exTranslate, translate) > tolerance) return false;
		if (Vec3.Distance(exScale,     scale)     > tolerance) return false;
		
		if (!mat.Decompose(out exTranslate, out exRotate, out exScale)) return false;
		Log.Info($"Matrix decompose:     {exTranslate} {exRotate} {exScale}");

		if (Vec3.Distance(exTranslate, translate) > tolerance) return false;
		if (Vec3.Distance(exScale,     scale)     > tolerance) return false;

		return true;
	}

	bool TestMatrixTransform()
	{
		Vec3 translate = new Vec3(10, 0, 0);
		Vec3 rotate    = new Vec3(0,90,0);
		Vec3 scale     = new Vec3(2,2,2);
		Matrix mat = Matrix.TRS(translate, Quat.FromAngles(rotate), scale);
		Log.Info($"Transform matrix: {translate} {rotate} {scale}");

		// Check ray transforms
		Ray ray1 = new Ray(Vec3.Zero, V.XYZ(0,0,1));
		Ray ray2 = new Ray(V.XYZ(1,0,0), V.XYZ(0,0,1));

		Ray tRay1 = mat.Transform(ray1);
		Ray tRay2 = mat * ray2;

		Log.Info($"Ray {ray1} -> {tRay1}");
		Log.Info($"Ray {ray2} -> {tRay2}");

		if (Vec3.Distance(tRay1.position,  V.XYZ(10,0,0))  > tolerance) return false;
		if (Vec3.Distance(tRay1.direction, V.XYZ(2,0,0))   > tolerance) return false;
		if (Vec3.Distance(tRay2.position,  V.XYZ(10,0,-2)) > tolerance) return false;
		if (Vec3.Distance(tRay2.direction, V.XYZ(2,0,0))   > tolerance) return false;

		// Check pose transforms
		Pose pose1 = Pose.Identity;
		Pose pose2 = new Pose(V.XYZ(1,0,0), Quat.FromAngles(0,90,0));

		Pose tPose1 = mat.Transform(pose1);
		Pose tPose2 = mat * pose2;

		Log.Info($"Pose {pose1} -> {tPose1}");
		Log.Info($"Pose {pose2} -> {tPose2}");

		if (Vec3.Distance(tPose1.position, V.XYZ(10, 0, 0))  > tolerance) return false;
		if (Vec3.Distance(tPose1.Forward,  V.XYZ(-1, 0, 0))  > tolerance) return false;
		if (Vec3.Distance(tPose2.position, V.XYZ(10, 0, -2)) > tolerance) return false;
		if (Vec3.Distance(tPose2.Forward,  V.XYZ(0, 0, 1))   > tolerance) return false;

		return true;
	}

	bool TestAngleDist()
	{
		float angleDistA = SKMath.AngleDist(0, 359);
		float angleDistB = SKMath.AngleDist(359, 0);
		float angleDistC = SKMath.AngleDist(359, 720);
		float angleDistD = SKMath.AngleDist(-60, 70);
		float angleDistE = SKMath.AngleDist(-60, 140);

		Log.Info($"AngleDist 0 to 359: {angleDistA}");
		Log.Info($"AngleDist 359 to 0: {angleDistB}");
		Log.Info($"AngleDist 359 to 720: {angleDistC}");
		Log.Info($"AngleDist -60 to 70: {angleDistD}");
		Log.Info($"AngleDist -60 to 140: {angleDistE}");

		if (MathF.Abs(angleDistA - angleDistB) > tolerance) return false;
		if (MathF.Abs(angleDistA - angleDistC) > tolerance) return false;
		if (MathF.Abs(angleDistA - 1)          > tolerance) return false;
		if (MathF.Abs(angleDistD - 130)        > tolerance) return false;
		if (MathF.Abs(angleDistE - 160)        > tolerance) return false;
		return true;
	}

	bool TestVector2Angles()
	{
		float angle43 = Vec2.AngleBetween(Vec2.FromAngle(0  ), Vec2.FromAngle(43));
		float angle16 = Vec2.AngleBetween(Vec2.FromAngle(27 ), Vec2.FromAngle(43));
		float angle74 = Vec2.AngleBetween(Vec2.FromAngle(117), Vec2.FromAngle(43));

		Log.Info($"Vec2.AngleBetween(Vec2.FromAngle(0  ), Vec2.FromAngle(43)): {angle43} - expected 43");
		Log.Info($"Vec2.AngleBetween(Vec2.FromAngle(27 ), Vec2.FromAngle(43)): {angle16} - expected 16");
		Log.Info($"Vec2.AngleBetween(Vec2.FromAngle(117), Vec2.FromAngle(43)): {angle74} - expected -74");

		if (MathF.Abs(angle43 - 43) > tolerance) return false;
		if (MathF.Abs(angle16 - 16) > tolerance) return false;
		if (MathF.Abs(angle74 + 74) > tolerance) return false;
		return true;
	}

	bool TestVector3Angles()
	{
		float angle43 = Vec3.AngleBetween(Vec3.AngleXZ(0  ), Vec3.AngleXZ(43));
		float angle16 = Vec3.AngleBetween(Vec3.AngleXZ(27 ), Vec3.AngleXZ(43));
		float angle74 = Vec3.AngleBetween(Vec3.AngleXY(117), Vec3.AngleXY(43));

		Log.Info($"Vec3.AngleBetween(Vec3.AngleXZ(0  ), Vec3.AngleXZ(43)): {angle43} - expected 43");
		Log.Info($"Vec3.AngleBetween(Vec3.AngleXZ(27 ), Vec3.AngleXZ(43)): {angle16} - expected 16");
		Log.Info($"Vec3.AngleBetween(Vec3.AngleXY(117), Vec3.AngleXY(43)): {angle74} - expected 74");

		if (MathF.Abs(angle43 - 43) > tolerance) return false;
		if (MathF.Abs(angle16 - 16) > tolerance) return false;
		if (MathF.Abs(angle74 - 74) > tolerance) return false;
		return true;
	}

	public void Initialize()
	{
		Tests.Test(TestMatrixDecompose);
		Tests.Test(TestMatrixTransform);
		Tests.Test(TestAngleDist);
		Tests.Test(TestVector2Angles);
		Tests.Test(TestVector3Angles);
	}

	public void Shutdown() { }

	public void Update() { }
}