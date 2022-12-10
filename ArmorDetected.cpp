#include<iostream>
#include<opencv2/opencv.hpp>

using namespace std;
using namespace cv;

class LightDescriptor 
{
	friend int main();
private:
	float width;
	float height;
	float angle;
	float area;
	Point2f center;
public:
	LightDescriptor() {}
	LightDescriptor(RotatedRect &light) 
	{	
		width = light.size.width;
		height = light.size.height;
		angle = light.angle;
		area = light.size.area();
		center = light.center;

	}
	const LightDescriptor& operator =(const LightDescriptor& ld)
	{
		this->width = ld.width;
		this->height = ld.height;
		this->center = ld.center;
		this->angle = ld.angle;
		this->area = ld.area;
		return *this;
	}
};
int main()
{
	VideoCapture video("D:/downloads/1.mp4");
	if (!video.isOpened())
	{
		cout << "failed to read the video" << endl;
		system("pause");
	}
	Mat frame, channels[3], binary, Gaussian, dilatee;
	Mat kernel = getStructuringElement(MORPH_RECT, Size(5, 5));
	vector<vector<Point> > contours;
	while (true)
	{
		video >> frame;
		if (frame.empty())
			break;
		//处理每一帧
		split(frame, channels);
		threshold(channels[2], binary, 220, 255, 0);//二值化
		GaussianBlur(binary, Gaussian, Size(3, 3), 0);//高斯滤波
		dilate(Gaussian, dilatee, kernel);//膨胀
		findContours(dilatee, contours, RETR_TREE, CHAIN_APPROX_NONE);
		vector<LightDescriptor> ld;
		for (unsigned int i = 0; i < contours.size(); i++)
		{
			double area = contourArea(contours[i]);
			if (area < 10 || contours[i].size() < 2)
			{
				continue;
			}
			RotatedRect lightRect = fitEllipse(contours[i]);//椭圆拟合
			if (lightRect.size.width / lightRect.size.height > 4)
			{
				continue;
			}
			ld.push_back(LightDescriptor(lightRect));
		}
		for (size_t i = 0; i < ld.size(); i++)
		{
			for (size_t j = i + 1; j < ld.size(); j++)
			{
				LightDescriptor& leftLight = ld[i];
				LightDescriptor& rightLight = ld[j];
				float deltAngle = abs(leftLight.angle - rightLight.angle);
				float heightRatio1 = abs(leftLight.height - rightLight.height) / max(leftLight.height, rightLight.height);
				float averageDistance = pow(pow((leftLight.center.x - rightLight.center.x), 2) + pow((leftLight.center.y - rightLight.center.y), 2), 0.5);
				float averageHeight = (leftLight.height + rightLight.height) / 2.0;
				float heightRatio2 = abs(leftLight.height - rightLight.height) / averageHeight;
				float delty = abs(leftLight.center.y - rightLight.center.y);
				float yRatio = delty / averageHeight;
				float deltx = abs(leftLight.center.x - rightLight.center.x);
				float xRatio = deltx / averageHeight;
				float ratio = averageDistance / averageHeight;
				//条件筛选
				if (deltAngle > 2 || heightRatio1 > 0.8 || heightRatio2 > 0.5 || yRatio > 1.2 || xRatio > 2.2 || xRatio < 0.8 || ratio>3 || ratio < 0.8)
				{
					continue;
				}
				float avgx = (leftLight.center.x + rightLight.center.x) / 2.0f;
				float avgy = (leftLight.center.y + rightLight.center.y) / 2.0f;
				float avgangle = (leftLight.angle + rightLight.angle) / 2.0f;
				Point center = Point(avgx, avgy);
				RotatedRect rrect = RotatedRect(center, Size(averageDistance, averageHeight), avgangle);
				Point2f points[4];
				rrect.points(points);
				for (int i = 0; i < 4; i++)
				{
					line(frame, points[i], points[(i + 1) % 4], Scalar(0, 255, 0), 2.2);
				}
			}
		}
		cv::imshow("frame", frame);
		cv::waitKey(30);
	}
	return 0;
}
