package com.gaejexperiments.db;

import java.io.IOException;
import java.util.Date;
import java.util.logging.Logger;

import javax.servlet.ServletException;
import javax.servlet.http.*;

@SuppressWarnings("serial")
public class PostSensorValueServlet extends HttpServlet {
	public static final Logger _logger = Logger
			.getLogger(PostSensorValueServlet.class.getName());

	@Override
	protected void doGet(HttpServletRequest req, HttpServletResponse resp)
			throws ServletException, IOException {
		doPost(req, resp);
	}

	public void doPost(HttpServletRequest req, HttpServletResponse resp)
			throws IOException {
		resp.setContentType("text/plain");
		String strResponse = "";
		String strTagCode = "";
		String strNodeID = "";
		String strSensor1Value = "";
		String strSensor2Value = "";
		String strSensor3Value = "";
		try {
			strNodeID = (String) req.getParameter("nodeid");
			strSensor1Value = (String) req.getParameter("sensor1value");
			strSensor2Value = (String) req.getParameter("sensor2value");
			strSensor3Value = (String) req.getParameter("sensor3value");
			strTagCode = (String) req.getParameter("tagcode");
			Date dt = new Date();
			SensorReport HR = new SensorReport(strNodeID,strTagCode,strSensor1Value,strSensor2Value,strSensor3Value, dt);
			DBUtils.saveSensorReport(HR);
			strResponse = "Sensor values has been stored successfully.";
		} catch (Exception ex) {
			_logger.severe("Error in saving Sensor Record : " + strTagCode
					+ "," + strTagCode + " : " + ex.getMessage());
			strResponse = "Error in saving Sensor Record via web. Reason : "
					+ ex.getMessage();
		}
		resp.getWriter().println(strResponse);
	}
}